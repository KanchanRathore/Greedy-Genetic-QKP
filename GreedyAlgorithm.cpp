#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>

struct Object {
    int id;
    double weight;
    double value;
    std::vector<double> pairwiseValues;
    double absoluteDensity;
    double relativeDensity;


    bool operator==(const Object& other) const {
        return id == other.id;
    }
};

// Parse input data from file
void parseInputData(const std::string& filename, int& numObjects, double& capacity, std::vector<Object>& objects, int& constraintType) {
    std::ifstream inputFile(filename);
    std::string line;

    // Skip the instance name
    std::getline(inputFile, line);

    // Read the number of objects
    inputFile >> numObjects;
    objects.resize(numObjects);

    // Read linear coefficients (c_i) for each object
    for (int i = 0; i < numObjects; ++i) {
        double value;
        inputFile >> value;
        objects[i] = {i, 0.0, value, std::vector<double>(numObjects, 0.0), 0.0, 0.0};
    }

    // Read quadratic coefficients (c_ij) in triangular matrix format
    for (int i = 0; i < numObjects - 1; ++i) {
        for (int j = i + 1; j < numObjects; ++j) {
            double pairValue;
            inputFile >> pairValue;
            objects[i].pairwiseValues[j] = pairValue;
            objects[j].pairwiseValues[i] = pairValue;
        }
    }

    // Skip blank line
    std::getline(inputFile, line);
    std::getline(inputFile, line);

    // Read constraint type (0 or 1) and capacity
    inputFile >> constraintType;
    inputFile >> capacity;

    // Read weights (a_i) for each object
    for (int i = 0; i < numObjects; ++i) {
        inputFile >> objects[i].weight;
    }
}

// Function to calculate the relative value density of an object with respect to the current solution
double calculateRelativeDensity(const Object& obj, const std::vector<Object>& selectedObjects) {
    double totalValue = obj.value;
    for (const auto& selected : selectedObjects) {
        totalValue += obj.pairwiseValues[selected.id];
    }
    return totalValue / obj.weight;
}

// Absolute Greedy Algorithm
std::vector<Object> absoluteGreedy(std::vector<Object>& objects, double capacity) {
    std::vector<Object> selectedObjects;
    double currentWeight = 0.0;

    // Calculate absolute density for each object
    for (auto& obj : objects) {
        obj.absoluteDensity = calculateRelativeDensity(obj, objects);
    }

    // Sort objects by absolute density in descending order
    std::sort(objects.begin(), objects.end(), [](const Object& a, const Object& b) {
        return a.absoluteDensity > b.absoluteDensity;
    });

     std::cout<<"Object ID Selcted"<<std::endl;

    // Select objects while respecting the capacity
    for (const auto& obj : objects) {
        if (currentWeight + obj.weight <= capacity) {
            selectedObjects.push_back(obj);
            currentWeight += obj.weight;

            std::cout <<obj.id<<" ";
        }
    }
    return selectedObjects;
}

// Function to find the best knapsack solution using the relative greedy algorithm
std::pair<std::vector<Object>, double> relativeGreedyKnapsack(const std::vector<Object>& objects, double capacity) {
    std::vector<Object> bestSolution;
    double bestValue = 0.0;
    double currentWeight;
    // Iterate over all objects as the starting point
    for (const auto& startObject : objects) {
        std::vector<Object> currentSolution;
        std::vector<Object> remainingObjects = objects;
        double currentWeight = 0.0;

        // Start with the current object
        currentSolution.push_back(startObject);
        currentWeight += startObject.weight;
        remainingObjects.erase(std::remove(remainingObjects.begin(), remainingObjects.end(), startObject), remainingObjects.end());

        // Iteratively add objects based on relative density
        while (true) {
            double maxDensity = -std::numeric_limits<double>::infinity();
            Object* bestObject = nullptr;

            // Find the object with the highest relative density that fits in the remaining capacity
            for (auto& obj : remainingObjects) {
                if (currentWeight + obj.weight <= capacity) {
                    double relativeDensity = calculateRelativeDensity(obj, currentSolution);
                    if (relativeDensity > maxDensity) {
                        maxDensity = relativeDensity;
                        bestObject = &obj;
                    }
                }
            }

            // If no valid object is found, stop the process
            if (!bestObject) break;

            // Add the best object to the current solution
            currentSolution.push_back(*bestObject);
            currentWeight += bestObject->weight;

            remainingObjects.erase(std::remove(remainingObjects.begin(), remainingObjects.end(), *bestObject), remainingObjects.end());
        }



        // Calculate the total value of the current solution
        double currentValue = 0.0;
        for (const auto& obj : currentSolution) {
            currentValue += obj.value;
        }
        for (size_t i = 0; i < currentSolution.size(); ++i) {
            for (size_t j = i + 1; j < currentSolution.size(); ++j) {
                currentValue += currentSolution[i].pairwiseValues[currentSolution[j].id];
            }
        }

        // Update the best solution if the current one is better
        if (currentValue > bestValue) {
            bestValue = currentValue;
            bestSolution = currentSolution;
        }
    }
    return {bestSolution, bestValue};
}

// Calculate total knapsack value for a given selection of objects
double calculateKnapsackValue(const std::vector<Object>& selectedObjects) {
    double totalValue = 0.0;

    // Sum individual values
    for (const auto& obj : selectedObjects) {
        totalValue += obj.value;
    }


    // Sum pairwise values for selected objects
    for (size_t i = 0; i < selectedObjects.size(); ++i) {
        for (size_t j = i + 1; j < selectedObjects.size(); ++j) {
            totalValue += selectedObjects[i].pairwiseValues[selectedObjects[j].id];
        }
    }


    return totalValue;
}

int main() {
     int numObjects, constraintType;
    std::vector<Object> objects;
    double capacity;

    // Load input data from a file
     parseInputData("input200_8.txt", numObjects, capacity, objects, constraintType);

     // Run Absolute Greedy Algorithm
    auto absoluteSolution = absoluteGreedy(objects, capacity);
    double absoluteValue = calculateKnapsackValue(absoluteSolution);
     std::cout<<"\n"<<std::endl;
    std::cout << "Absolute Greedy Knapsack Value: " << absoluteValue << "\n";

    // Run the relative greedy knapsack algorithm
    auto [solution, value] = relativeGreedyKnapsack(objects, capacity);

    // Output the solution
    std::cout << "Best solution (object IDs): ";
    for (const auto& obj : solution) {
        std::cout << obj.id << " ";
    }
    std::cout<<"\n"<<std::endl;
    std::cout<<"Relative Greedy Knapsack Value :";
    std::cout << value << std::endl;

    return 0;
}
