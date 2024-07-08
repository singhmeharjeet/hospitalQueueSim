#include <cmath>
#include <iostream>
#include <queue>
#include <random>

// Found this function on stackoverflow.com, used for debugging purposes
// && printing to console
void println() { std::cout << "\n"; }

template <typename Arg, typename... Args>
void println(Arg &&arg, Args &&...args) {
	std::cout.precision(4);
	std::cout.flags(std::ios::fixed);
	std::cout << std::forward<Arg>(arg);
	using expander = int[];
	(void)expander{0, (void(std::cout << " : " << std::forward<Args>(args)), 0)...};
	std::cout << "\n";
}
/**
 * Global namespace for variables
 * 1) Inputs (Stores all the input values from the console)
 * 2) Stats (Stores all the statistics that need to be calculated)
 * 3) Random (Stores the random number generator and distribution)
 * 4) Time (stores all the different times)
 */
namespace Inputs {
double lambda = 0;
double mu_e = 0;
double mu_t = 0;
double mu_c = 0;
int B = 0;
int R = 0;
int m1 = 0;
int m2 = 0;
int seed = 0;
}  // namespace Inputs
namespace Stats {
double totalPatients = 0;
int departures = 0;
int lostArrivals = 0;
int busy_nurses = 0;
int busy_rooms = 0;
int busy_janitors = 0;

double cumulativeWaitTimeEvaluation = 0;
double cumulativeWaitTimeTreatment = 0;
double cumulativeCleanupTime = 0;
double cumulativeResponseTime = 0;
}  // namespace Stats

namespace Random {
std::default_random_engine generator(Inputs::seed);
std::uniform_real_distribution<double> distribution(0.0, 1.0);

double uniform() {
	return distribution(generator);	 // [0, 1)
}

double exponential(double rate) {
	return -log(1 - uniform()) / rate;
}
}  // namespace Random

namespace Time {
double currentTime = 0;
double resetTime = 60 * 6;	// 6 hours
double endTime = 60 * 30;	// 30 hours
}  // namespace Time

struct Patient {
	static int total_patient_count;
	int id;	 //(in serial order) used for debugging && comparing which patient arrived first
	double severity;
	double arrivalTime;
	double evaluationDuration;
	double transferTime;
	double treatmentDuration;
	double departureTime;
	double cleanupDuration;

	/**
	 * Constructor for Patient:
	 * 1) Assigns a unique id to the patient which is in serial order [1, 2, 3, ...]
	 * 2) Assigns a severity, evaluation duration, treatment duration and cleanup duration using random numbers.
	 * @param arrivalTime
	 */
	explicit Patient(double arrivalTime) {
		id = total_patient_count++;
		this->arrivalTime = arrivalTime;
		evaluationDuration = Random::exponential(Inputs::mu_e);
		transferTime = 0;
		severity = Random::uniform();
		treatmentDuration = severity < 0.5 ? Random::exponential(Inputs::mu_t) : Random::exponential(Inputs::mu_t * 2);
		departureTime = 0;
		cleanupDuration = 1 / Inputs::mu_c;
	}
};

int Patient::total_patient_count = 1;

enum EventType {
	ARRIVAL,	 // push to eval queue
	TRANSFER,	 // pop from eval queue, push to treat queue
	DEPARTURE,	 // pop from treat queue, push to clean queue
	END_CLEANUP	 // pop from clean queue
};

struct Event {
	EventType type;
	double time;
	Patient patient;

	Event() = delete;

	Event(const EventType type, const double time, const Patient &patient) : type(type), time(time), patient(patient) {}
};

/**
 * Comparator for the priority queue of patients in the treatment queue
 */
struct PatientCompare {
	bool operator()(const Patient &p, const Patient &q) const {
		return p.severity < q.severity;
	}
};

/**
 * Comparator for the priority queue of events
 */
struct EventCompare {
	bool operator()(const Event &e1, const Event &e2) const {
		if (e1.time == e2.time && e1.patient.id != e2.patient.id) {
			// same time, different patient => priority is given to patient who arrived first
			return e1.patient.id > e2.patient.id;
		}
		return e1.time > e2.time;
	}
};

/**
 * Simulation class:
 * 1) Contains
 * 		- event list (priority queue => sorted by ascending time => range [0, inf])
 * 		- evaluation queue (deque => FIFO)
 * 		- treatment queue (priority queue => sorted by highest severity first => range [0, 1])
 * 		- cleanup queue (deque => FIFO) (only used to keep track of waiting times)
 * 2) Contains all the functions that are used to process the events and change the state of queues
 */
class Sim {
	std::priority_queue<Event, std::vector<Event>, EventCompare> eventList;
	std::deque<Patient> eval_queue;
	std::priority_queue<Patient, std::vector<Patient>, PatientCompare> treat_queue;
	std::deque<Patient> clean_queue;

	public:
	Sim() = delete;

	Sim(const Sim &s) = delete;

	Sim &operator=(const Sim &s) = delete;

	~Sim() = default;

	Sim(double lambda, double mu_e, double mu_t, double mu_c, int B, int R, int m1, int m2, int seed);

	void run();

	void processArrival(Event &e);

	void processEvaluation();

	void processTransfer(Event &e);

	void processTreatment();

	void processDeparture(Event &e);

	void processCleaning();

	void processEndCleaning(Event &e);

	void printStats();
};

/**
 * Constructor for Simulation:
 * 1) Assigns all the input values to the corresponding global variables
 * 2) Initializes the random number generator
 * 3) Initializes the event list with an arrival event at time 0
 */
Sim::Sim(double lambda, double mu_e, double mu_t, double mu_c, int B, int R, int m1, int m2, int seed) {
	Inputs::lambda = lambda;
	Inputs::mu_e = mu_e;
	Inputs::mu_t = mu_t;
	Inputs::mu_c = mu_c;
	Inputs::B = B;
	Inputs::R = R;
	Inputs::m1 = m1;
	Inputs::m2 = m2;
	Inputs::seed = seed;
	Random::generator.seed(Inputs::seed);
	eventList.emplace(ARRIVAL, 0, Patient(0));
}

/**
 * Runs the simulation until the end time is reached
 * 1) Gets the next event from the event list
 * 2) Passes that event to the appropriate function
 * 3) These functions will update the
 * 		- event list
 * 		- evaluation and treatment queues
 * 		- statistics of the simulation
 * 4) Prints the statistics of the simulation
 */
void Sim::run() {
	while (Time::currentTime < Time::endTime) {
		Event e = eventList.top();
		eventList.pop();
		Time::currentTime = e.time;
		switch (e.type) {
		case ARRIVAL:
			processArrival(e);
			break;
		case TRANSFER:
			processTransfer(e);
			break;
		case DEPARTURE:
			processDeparture(e);
			break;
		case END_CLEANUP:
			processEndCleaning(e);
			break;
		}
	}
	printStats();
}

/**
 * Processes an arrival event:
 * 1) Schedule arrival of the next patient to keep the simulation running.
 * 2) Checks the system capacity before admitting the patient
 * 		- If the system is at capacity, the patient is lost
 * 3) Schedules the evaluation of the patient on top of the waiting list
 * 		- If any nurse is free, the evaluation starts immediately
 * 			(of the top patient in the evaluation queue AND NOT the current patient)
 * 		- Else the patient remains in the evaluation queue (and will be evaluated when a nurse is free)
 * @param e contains the patient who arrived
 */
void Sim::processArrival(Event &e) {
	double nextArrival = Time::currentTime + Random::exponential(Inputs::lambda);
	eventList.emplace(ARRIVAL, nextArrival, Patient(nextArrival));

	if (Stats::totalPatients >= Inputs::B) {
		if (Time::currentTime > Time::resetTime) Stats::lostArrivals++;
		return;
	}

	Stats::totalPatients++;
	e.patient.arrivalTime = Time::currentTime;
	eval_queue.push_back(e.patient);

	if (Stats::busy_nurses < Inputs::m1)
		processEvaluation();
}

/**
 * Processes evaluation of a patient:
 * 1) Removes the patient from the evaluation queue
 * 2) Schedules the transfer of the patient to the treatment queue
 */
void Sim::processEvaluation() {
	Stats::busy_nurses++;
	Patient p = eval_queue.front();
	eval_queue.pop_front();

	if (Time::currentTime > Time::resetTime)
		Stats::cumulativeWaitTimeEvaluation += Time::currentTime - p.arrivalTime;

	eventList.emplace(TRANSFER, Time::currentTime + p.evaluationDuration, p);
}

/**
 * Processes transfer of a patient from evaluation queue to treatment queue:
 * 1) Adds the patient to the treatment queue (patient is already removed from the evaluation queue)
 * 2) Since, a nurse gets free, it schedules the evaluation of the top() patient in eval queue
 * 3) Schedules the treatment of the top patient in the treatment queue
 * 		- If any room is free, the treatment starts immediately
 * 		- Else the patient remains in the treatment queue (and will be treated when a room is free)
 * @param e contains the patient to be transferred
 */
void Sim::processTransfer(Event &e) {
	Stats::busy_nurses--;

	e.patient.transferTime = Time::currentTime;
	treat_queue.push(e.patient);

	if (Stats::busy_rooms < Inputs::R)
		processTreatment();
	if (!eval_queue.empty())
		processEvaluation();
}

/**
 * Processes treatment of a patient:
 * 1) Removes the patient from the treatment queue
 * 2) Schedules the departure of the patient
 */
void Sim::processTreatment() {
	Stats::busy_rooms++;
	Patient p = treat_queue.top();
	treat_queue.pop();

	if (Time::currentTime > Time::resetTime)
		Stats::cumulativeWaitTimeTreatment += Time::currentTime - p.transferTime;

	eventList.emplace(DEPARTURE, Time::currentTime + p.treatmentDuration, p);
}

/**
 * Processes departure of a patient:
 * 1) Removes the patient from the system
 * 		(only decrements the total number of patients as the patient is already removed from the treatment queue)
 * 2) Add the room which the patient was occupying, to the list of cleanup rooms
 * 		(The patient is added to the cleanup queue as we need to keep track of the time the patient left)
 * 3) Start the cleanup process
 * 		- immediately if any janitor is free
 * 		- else the room waits in the cleanup queue
 * @param e contains the patient to be removed from the system
 */
void Sim::processDeparture(Event &e) {
	Stats::totalPatients--;

	if (Time::currentTime > Time::resetTime) {
		Stats::departures++;
		Stats::cumulativeResponseTime += Time::currentTime - e.patient.arrivalTime;
	}

	e.patient.departureTime = Time::currentTime;
	clean_queue.push_back(e.patient);

	if (Stats::busy_janitors < Inputs::m2)
		processCleaning();
}

/**
 * Processes cleaning of a room:
 * 1) Removes the room from the cleanup queue
 * 2) Schedules the end of the cleanup process
 */
void Sim::processCleaning() {
	Stats::busy_janitors++;
	Patient p = clean_queue.front();
	eventList.emplace(END_CLEANUP, Time::currentTime + p.cleanupDuration, p);
	clean_queue.pop_front();
}

/**
 * Processes the end of the cleanup process:
 * 1) Decrements the number of busy janitors and rooms
 * 2) Because now we have at least 1 room and 1 janitor free, the function checks for
 * 		- Waiting patients in the treatment queue (if any, then start treatment)
 * 		- Waiting rooms in the cleanup queue (if any, then start cleanup)
 * @param e
 */
void Sim::processEndCleaning(Event &e) {
	Stats::busy_janitors--;
	Stats::busy_rooms--;

	if (Time::currentTime > Time::resetTime)
		Stats::cumulativeCleanupTime += Time::currentTime - e.patient.departureTime;

	if (!clean_queue.empty())
		processCleaning();
	if (!treat_queue.empty())
		processTreatment();
}

void Sim::printStats() {
	println();
	println("Departures", Stats::departures);

	if (Stats::departures == 0) {
		Stats::departures = 1;					  // Avoid division by zero
		Stats::cumulativeWaitTimeEvaluation = 0;  // To resolve BUG: it is 'nan' otherwise ??
	}

	println("Mean_num_patients ", Stats::cumulativeResponseTime / (Time::endTime - Time::resetTime));
	println("Mean_response_time", Stats::cumulativeResponseTime / Stats::departures);
	println("Mean_wait_E_queue ", Stats::cumulativeWaitTimeEvaluation / Stats::departures);
	println("Mean_wait_P_queue ", Stats::cumulativeWaitTimeTreatment / Stats::departures);
	println("Mean_cleanup_time ", Stats::cumulativeCleanupTime / Stats::departures);
	println("Dropped_arrivals  ", Stats::lostArrivals);
}

int main(int argc, char *argv[]) {
	if (argc != 10) {
		std::cout << "Invalid number of arguments" << std::endl;
		return 1;
	}

	double lambda = std::stod(argv[1]);

	double mu_e = std::stod(argv[2]);
	double mu_t = std::stod(argv[3]);
	double mu_c = std::stod(argv[4]);
	int B = std::stoi(argv[5]);
	int R = std::stoi(argv[6]);
	int m1 = std::stoi(argv[7]);
	int m2 = std::stoi(argv[8]);
	int S = std::stoi(argv[9]);

	if (lambda < 0 || mu_e <= 0 || mu_t <= 0 || mu_c <= 0 || B <= 0 || R <= 0 || m1 <= 0 || m2 <= 0 || S <= 0) {
		std::cout << "Invalid input" << std::endl;
		return 1;
	}

	Sim sim(lambda, mu_e, mu_t, mu_c, B, R, m1, m2, S);
	sim.run();
}
