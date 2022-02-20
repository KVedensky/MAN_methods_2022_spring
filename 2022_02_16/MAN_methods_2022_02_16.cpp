#include <vector>
#include <iostream>
#include <tuple>
#include <algorithm>
#include <set>

const int randseed = 1;

// Number of floors, including the ground floor
// Ground floor is 1, top floor is NumFloors
const int NumFloors = 10;

// Exactly the same on each floor
const int NumFlatsPerFloor = 10;

// Average number of people per flat - should be random
const int AvgPeoplePerFlat = 2;

//NumElevators = 1
const int SecondsBetweenFloors = 5;
const int SecondsOnFloor = 10;
const int MaxCapacity = 5;

const int DebugLevel = 0;

using namespace std;

vector<tuple<int, int, int> > ElevatorCalls;
int ElevatorCallsIdx;
int CurrentSecond;
vector<tuple<int, int, int> > CurrentCalls;
int ElevatorPosition;

// Generates elevator calls for people from a single flat
// TODO(vedensky) : add types of flats
// TODO(vedensky) : one person per flat works 09 - 18
vector<tuple<int, int, int> > GenerateCallsForFlat(int numPeople, int floor) {
	vector<tuple<int, int, int> > result;
	for (int i = 0; i < numPeople; i++) {
		int time1 = (rand()*RAND_MAX + rand()) % (22 * 60 * 60);
		result.push_back(make_tuple(time1, floor, 1));
		int time2 = time1 + 60 * 60 + (rand() * RAND_MAX + rand()) % (24 * 60 * 60 - (time1 + 60 * 60));
		result.push_back(make_tuple(time2, 1, floor));
	}
	return result;
}

// TODO(vedensky) : make the number of people random
vector<tuple<int, int, int> > GetPeopleArrivingAtFloors(int numFloors, int numFlatsPerFloor, int avgPeoplePerFlat) {
	vector<tuple<int, int, int> > result;
	for (int floor = 2; floor <= numFloors; floor++) {
		for (int flat = 0; flat < numFlatsPerFloor; flat++) {
			auto new_calls = GenerateCallsForFlat(avgPeoplePerFlat, floor);
			result.insert(result.end(), new_calls.begin(), new_calls.end());
		}
	}
	return result;
}

vector<tuple<int, int, int> > GenerateElevatorCalls() {
	vector<tuple<int, int, int> > result = GetPeopleArrivingAtFloors(NumFloors, NumFlatsPerFloor, AvgPeoplePerFlat);
	sort(result.begin(), result.end());
	result.push_back(make_tuple(60 * 60 * 60, -1, -1));
	return result;
}

vector<tuple<int, int, int> > GetCallsUpToCurrentSecond() {
	vector<tuple<int, int, int> > result;
	while (get<0>(ElevatorCalls[ElevatorCallsIdx]) <= CurrentSecond) {
		result.push_back(ElevatorCalls[ElevatorCallsIdx]);
		ElevatorCallsIdx++;
	}
	return result;
}

void MoveElevatorToFloor(int targetFloor) {
	if (DebugLevel >= 1)
		cout << "Second: " << CurrentSecond << " - Elevator moved from floor " << ElevatorPosition << " to floor " << targetFloor << endl;
	CurrentSecond += SecondsBetweenFloors * abs(targetFloor - ElevatorPosition);
	ElevatorPosition = targetFloor;
}

bool IsCalledFromCurrentFloor() {
	for (auto call : CurrentCalls)
		if (get<1>(call) == ElevatorPosition)
			return true;
	return false;
}

void PrintCalls(vector<tuple<int, int, int> > calls) {
	for (auto call : calls) {
		cout << "(" << get<0>(call) << ", " << get<1>(call) << ", " << get<2>(call) << ")" << endl;
	}
}

int main() {
	srand(randseed);
	ElevatorCalls = GenerateElevatorCalls();
	cout << ElevatorCalls.size() << endl;

	CurrentSecond = 0;
	ElevatorPosition = 1;

	ElevatorCallsIdx = 0;

	if (DebugLevel >= 1) {
		PrintCalls(ElevatorCalls);
		cout << "Start: second 0, elevator is at the floor 1, empty." << endl;
	}

	vector<int> WaitTimes;

	while ((ElevatorCallsIdx != ElevatorCalls.size() - 1) || (CurrentCalls.size() != 0)) {
		if (CurrentCalls.size() == 0)
			CurrentSecond = get<0>(ElevatorCalls[ElevatorCallsIdx]);
		auto calls = GetCallsUpToCurrentSecond();
		CurrentCalls.insert(CurrentCalls.end(), calls.begin(), calls.end());
		if (IsCalledFromCurrentFloor()) {
			CurrentSecond += SecondsOnFloor;
			calls = GetCallsUpToCurrentSecond();
			CurrentCalls.insert(CurrentCalls.end(), calls.begin(), calls.end());
			vector<tuple<int, int, int> > MomentCalls;
			for (auto call : CurrentCalls)
				if (get<1>(call) == ElevatorPosition) {
					MomentCalls.push_back(call);
					if (MomentCalls.size() == MaxCapacity)
						break;
				}

			if (DebugLevel >= 1) {
				cout << "Second: " << CurrentSecond << " - Elevator picked up: " << endl;
				PrintCalls(MomentCalls);
			}
			int maxFloor = 1;
			set<int> setFloors;
			for (auto call : MomentCalls) {
				WaitTimes.push_back(CurrentSecond - get<0>(call));
				setFloors.insert(get<2>(call));
				maxFloor = max(maxFloor, get<2>(call));
				for (int i = 0; i < CurrentCalls.size(); i++) if (CurrentCalls[i] == call) {
					CurrentCalls.erase(CurrentCalls.begin() + i);
					break;
				}
			}
			if (DebugLevel >= 1)
				if (setFloors.size() - 1 > 0)
					cout << "Second: " << CurrentSecond << " - Elevator unloads at " << setFloors.size() - 1 << " mid floors" << endl;
			MoveElevatorToFloor(maxFloor);
			CurrentSecond += (setFloors.size() - 1) * SecondsOnFloor;
			if (DebugLevel >= 1)
				cout << "Second: " << CurrentSecond << " - Elevator unloads at floor " << ElevatorPosition << endl;
			CurrentSecond += SecondsOnFloor;
			calls = GetCallsUpToCurrentSecond();
			CurrentCalls.insert(CurrentCalls.end(), calls.begin(), calls.end());
			if (IsCalledFromCurrentFloor()) {
				CurrentSecond -= SecondsOnFloor;
				continue;
			}
		} else {
			MoveElevatorToFloor(get<1>(CurrentCalls[0]));
			continue;
		}
	}

	int avgWaitTime = 0;
	int maxWaitTime = 0;
	for (auto t : WaitTimes) {
		avgWaitTime += t;
		maxWaitTime = max(maxWaitTime, t);
	}
	cout << "Average wait time: " << avgWaitTime*1.0 / WaitTimes.size() << endl;
	cout << "Max wait time: " << maxWaitTime << endl;

	return 0;
}