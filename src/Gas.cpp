//
// Created by lobis on 28/09/2022.
//

#include "Gas.h"

#include <algorithm>
#include <iostream>

using namespace std;
using namespace Garfield;

Gas::Gas() : gas(make_unique<MediumMagboltz>()) {}

Gas::Gas(const string& gasFilepath) : gas(make_unique<MediumMagboltz>()) { gas->LoadGasFile(gasFilepath); }

Gas::Gas(const vector<string>& _components,
         const vector<double>& _fractions) {
    vector<std::string> components = _components;
    vector<double> fractions = _fractions;

    if (components.empty()) {
        cerr << "cannot initialize gas with empty components" << endl;
        exit(1);
    }
    if (components.size() == 1 && fractions.empty()) {
        fractions.emplace_back(1.0);
    }
    if (components.size() != fractions.size()) {
        cerr << "number of component names and fractions mismatch" << endl;
        exit(1);
    }

    // TODO: Throw when negative number as fraction, remove elements when fraction is zero
    const size_t componentsLimit = 6;
    if (components.size() > componentsLimit) {
        cerr << "cannot initialize gas more than 6 components" << endl;
        exit(1);
    }

    double fractionSum = 0;
    for (const auto& value: fractions) { fractionSum += value; }
    // normalize so the sum is 1
    for (auto& value: fractions) { value /= fractionSum; }

    while (components.size() < componentsLimit) {
        components.emplace_back("");
        fractions.emplace_back(0);
    }

    for (int i = 0; i < 6; i++) {
        cout << "component: " << components[i] << " fraction: " << fractions[i] << endl;
    }
    gas = make_unique<MediumMagboltz>(components[0], fractions[0], components[1], fractions[1], components[2], fractions[2], components[3], fractions[3], components[4], fractions[4], components[5], fractions[5]);
}

template<typename T>
std::vector<T> linspace(T start, T end, size_t points) {
    std::vector<T> result(points);
    float step = (end - start) / (points - 1);
    size_t i = 0;
    for (auto& value: result) {
        value = start + step * T(i++);
    }
    return result;
}

void Gas::Generate() {
    const auto electricField = linspace<double>(0, 300, 20);
    gas->SetFieldGrid(electricField, {0.0}, {HalfPi});

    const int numCollisions = 10;
    gas->GenerateGasTable(numCollisions);
}

void Gas::Write(const string& filename) const {
    gas->WriteGasFile(filename);
}

constexpr double torrToBar = 0.001333223684;
double Gas::GetPressure() const {
    return gas->GetPressure() * torrToBar;
}

void Gas::SetPressure(double pressureInBar) {
    gas->SetPressure(pressureInBar / torrToBar);
}

double Gas::GetTemperature() const {
    return gas->GetTemperature() - ZeroCelsius;
}

void Gas::SetTemperature(double temperatureInCelsius) {
    gas->SetTemperature(temperatureInCelsius + ZeroCelsius);
}

double Gas::GetElectronDriftVelocity(double electricField) const {
    double vx, vy, vz;
    gas->ElectronVelocity(0, 0, -electricField, 0, 0, 0, vx, vy, vz);
    return vz;
}

pair<double, double> Gas::GetElectronDiffusion(double electricField) const {
    double longitudinal, transversal;
    gas->ElectronDiffusion(0, 0, -electricField, 0, 0, 0, longitudinal, transversal);
    return {longitudinal, transversal};
}

double Gas::GetElectronTransversalDiffusion(double electricField) const {
    return GetElectronDiffusion(electricField).second;
}

double Gas::GetElectronLongitudinalDiffusion(double electricField) const {
    return GetElectronDiffusion(electricField).first;
}

double Gas::GetElectronTownsend(double electricField) const {
    double townsend;
    gas->ElectronTownsend(0, 0, -electricField, 0, 0, 0, townsend);
    return townsend;
}

double Gas::GetElectronAttachment(double electricField) const {
    double attachment;
    gas->ElectronAttachment(0, 0, -electricField, 0, 0, 0, attachment);
    return attachment;
}