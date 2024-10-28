#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <bitset>
#include <cstdlib>
#include <iomanip>

void printUsage() {
    std::cerr << "Usage: <program_name> <IRQ_NUMBER> <CPU_NUMBER>" << std::endl;
    std::cerr << "Author: Liansixin" << std::endl;
}

bool directoryExists(const std::string& path) {
    std::ifstream dir(path);
    return dir.good();
}

void printAvailableIRQs() {
    std::ifstream interruptsFile("/proc/interrupts");
    std::string line;
    std::cout << "Available IRQs are:" << std::endl;
    while (std::getline(interruptsFile, line)) {
        std::istringstream iss(line);
        std::string irqNumber;
        iss >> irqNumber;
        if (irqNumber.back() == ':') {
            irqNumber.pop_back(); // Remove trailing ':'
            try {
                int irq = std::stoi(irqNumber);
                std::cout << irq << std::endl;
            } catch (std::invalid_argument&) {
                // Skip non-integer values
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage();
        return 1;
    }

    int irqNumber, cpuNumber;
    try {
        irqNumber = std::stoi(argv[1]);
        cpuNumber = std::stoi(argv[2]);
    } catch (std::invalid_argument&) {
        std::cerr << "Invalid IRQ or CPU number." << std::endl;
        return 1;
    }

    // Check if IRQ directory exists
    std::string irqPath = "/proc/irq/" + std::to_string(irqNumber);
    if (!directoryExists(irqPath)) {
        std::cerr << "IRQ " << irqNumber << " does not exist." << std::endl;
        printAvailableIRQs();
        return 1;
    }

    // Calculate CPU affinity mask
    unsigned int affinityMask = 1 << cpuNumber;
    std::stringstream ss;
    ss << std::hex << affinityMask;
    std::string affinityMaskHex = ss.str();

    // Set IRQ affinity
    std::string affinityFile = irqPath + "/smp_affinity";
    std::ofstream ofs(affinityFile);
    if (!ofs) {
        std::cerr << "Failed to open " << affinityFile << " for writing." << std::endl;
        return 1;
    }
    ofs << affinityMaskHex;
    ofs.close();
    std::cout << "Setting IRQ " << irqNumber << " to CPU " << cpuNumber
              << " (affinity mask: 0x" << affinityMaskHex << ")" << std::endl;

    // Verify the setting
    std::ifstream ifs(affinityFile);
    std::string currentAffinity;
    if (ifs) {
        ifs >> currentAffinity;
        std::cout << "Current affinity for IRQ " << irqNumber << ": " << currentAffinity << std::endl;
    } else {
        std::cerr << "Failed to read " << affinityFile << " to verify setting." << std::endl;
    }

    return 0;
}
