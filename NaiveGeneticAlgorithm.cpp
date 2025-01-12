#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <random>
#include <ctime>
#include <limits>
using namespace std;

struct QKP {
    vector<int> weights;
    vector<int> values;
    vector<vector<int>> pairwise_values;
    int capacity;
    int num_objects;

    QKP(const vector<int>& w, const vector<int>& v, const vector<vector<int>>& p, int c)
        : weights(w), values(v), pairwise_values(p), capacity(c), num_objects(w.size()) {}

    int fitness(const vector<int>& chromosome) const {
        int total_value = 0;
        int total_weight = 0;
        for (int i = 0; i < num_objects; ++i) {
            if (chromosome[i] == 1) {
                total_value += values[i];
                total_weight += weights[i];
                for (int j = i + 1; j < num_objects; ++j) {
                    if (chromosome[j] == 1) {
                        total_value += pairwise_values[i][j];
                    }
                }
            }
        }
        return (total_weight > capacity) ? 0 : total_value;
    }
};

QKP parse_input_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file");
    }

    string line;
    getline(file, line);
    getline(file, line);
    int num_objects = stoi(line);

    getline(file, line);
    istringstream linear_coeffs_stream(line);
    vector<int> values;
    for (int i = 0; i < num_objects; ++i) {
        int value;
        linear_coeffs_stream >> value;
        values.push_back(value);
    }

    vector<vector<int>> pairwise_values(num_objects, vector<int>(num_objects, 0));
    for (int i = 0; i < num_objects - 1; ++i) {
        getline(file, line);
        istringstream quad_coeffs_stream(line);
        for (int j = i + 1; j < num_objects; ++j) {
            int pairwise_value;
            quad_coeffs_stream >> pairwise_value;
            pairwise_values[i][j] = pairwise_value;
            pairwise_values[j][i] = pairwise_value;
        }
    }

    getline(file, line);
    getline(file, line);
    getline(file, line);

    int capacity = stoi(line);
    getline(file, line);
    istringstream weights_stream(line);
    vector<int> weights;
    for (int i = 0; i < num_objects; ++i) {
        int weight;
        weights_stream >> weight;
        weights.push_back(weight);
    }

    return QKP(weights, values, pairwise_values, capacity);
}

vector<vector<int>> initialize_population(const QKP& problem, int population_size, mt19937& gen) {
    vector<vector<int>> population;

    for (int i = 0; i < population_size; ++i) {
        vector<int> chromosome(problem.num_objects, 0);
        vector<int> indices(problem.num_objects);
        iota(indices.begin(), indices.end(), 0);
        shuffle(indices.begin(), indices.end(), gen);

        int total_weight = 0;
        for (int idx : indices) {
            if (total_weight + problem.weights[idx] <= problem.capacity) {
                chromosome[idx] = 1;
                total_weight += problem.weights[idx];
            }
        }
        population.push_back(chromosome);
    }

    return population;
}

vector<int> tournament_selection(const vector<vector<int>>& population, const QKP& problem, int k, mt19937& gen) {
    double winnerProb = 90;
    std::vector<int> best_chromosome;
    std::vector<int> indices(population.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), gen);


    std::vector<int> tournamentGroup;
    for (int i = 0; i < k && i < indices.size(); ++i) {
        tournamentGroup.push_back(indices[i]);
    }
    int bestIdx = tournamentGroup[0];
    int bestFitness = problem.fitness(population[bestIdx]);
    int secondIdx = -1, secondFitness = -1;

    for (int i = 1; i < tournamentGroup.size(); ++i) {
        int idx = tournamentGroup[i];
        int fitness = problem.fitness(population[idx]);
        if (fitness > bestFitness) {
            secondIdx = bestIdx;
            secondFitness = bestFitness;
            bestIdx = idx;
            bestFitness = fitness;
        } else if (fitness > secondFitness) {
            secondIdx = idx;
            secondFitness = fitness;
        }
    }

    std::uniform_int_distribution<> dist(0, 100);
    int randomProb = dist(gen);
    int id = (randomProb < winnerProb) ? bestIdx : secondIdx;
    const auto &chromosome = population[id];
    return chromosome;
}

vector<int> crossover(const vector<int>& parent1, const vector<int>& parent2, const QKP& problem, mt19937& gen) {
    vector<int> offspring(problem.num_objects, 0);

    for (int i = 0; i < problem.num_objects; ++i) {
        if (parent1[i] == 1 && parent2[i] == 1) {
            offspring[i] = 1;
        }
    }

    vector<int> indices(problem.num_objects);
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), gen);

    int total_weight = 0;
    for (int idx = 0; idx < problem.num_objects; ++idx) {
        if (offspring[idx] == 1) {
            total_weight += problem.weights[idx];
        }
    }

    for (int idx : indices) {
        if (offspring[idx] == 0 && total_weight + problem.weights[idx] <= problem.capacity) {
            offspring[idx] = 1;
            total_weight += problem.weights[idx];
        }
    }

    return offspring;
}


vector<int> mutate(const vector<int>& parent, const QKP& problem, mt19937& gen) {
    vector<int> offspring = parent;
    vector<int> included, excluded;

    for (int i = 0; i < problem.num_objects; ++i) {
        if (offspring[i] == 1) included.push_back(i);
        else excluded.push_back(i);
    }

    uniform_real_distribution<> prob_dist(0.0, 1.0);
    for (int idx : included) {
        if (prob_dist(gen) < 2.0 / included.size()) {
            offspring[idx] = 0;
        }
    }

    shuffle(excluded.begin(), excluded.end(), gen);
     int total_weight = 0;
    for (int i = 0; i < problem.num_objects; ++i) {
        if (offspring[i] == 1) {
            total_weight += problem.weights[i];
        }
    }

    for (int idx : excluded) {
        if (total_weight + problem.weights[idx] <= problem.capacity) {
            offspring[idx] = 1;
            total_weight += problem.weights[idx];
        }
    }

    return offspring;
}

int run_genetic_algorithm(QKP& problem, int population_size, int generations) {
    random_device rd;
    mt19937 gen(rd());
    auto population = initialize_population(problem, population_size, gen);
    vector<int> best_chromosome;
    int best_fitness = numeric_limits<int>::min();

    for (int gen_num = 0; gen_num < generations; ++gen_num) {
        vector<vector<int>> new_population;

        if (gen_num > 0) new_population.push_back(best_chromosome);

        for (int i = (gen_num > 0 ? 1 : 0); i < population_size; ++i) {
            auto parent1 = tournament_selection(population, problem, 2, gen);
            auto parent2 = tournament_selection(population, problem, 2, gen);

            vector<int> offspring = (rand() % 100 < 70) ? crossover(parent1, parent2, problem, gen) : mutate(parent1, problem, gen);
            new_population.push_back(offspring);

            int offspring_fitness = problem.fitness(offspring);
            if (offspring_fitness > best_fitness) {
                best_fitness = offspring_fitness;
                best_chromosome = offspring;
            }
        }

        population = new_population;
        std::cout << "Generation " << gen_num + 1 << ": Best Fitness = " << best_fitness << std::endl;
    }

    return best_fitness;
}

void run_multiple_trials(QKP& problem, int population_size, int generations, int num_trials) {
    vector<int> trial_results;
    for (int trial = 0; trial < num_trials; ++trial) {
        cout << "Running Trial " << trial + 1 << "..." << endl;
        int best_fitness = run_genetic_algorithm(problem, population_size, generations);
        trial_results.push_back(best_fitness);
        cout << "Trial " << trial + 1 << ": Best Fitness = " << best_fitness << endl;
    }

    double mean_fitness = accumulate(trial_results.begin(), trial_results.end(), 0.0) / trial_results.size();
    cout << "\nMean Fitness Over " << num_trials << " Trials = " << mean_fitness << endl;
}

int main() {
    try {
        QKP problem = parse_input_file("input1.txt");

        int population_size = problem.num_objects;
        int generations = 10 * problem.num_objects;
        int num_trials = 50;

        run_multiple_trials(problem, population_size, generations, num_trials);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

