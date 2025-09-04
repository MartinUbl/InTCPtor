/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains configuration settings for the library.
 */

#include "config.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

constexpr bool Debug_Config_Outputs = false;

const std::string Config_Filename = "intcptor_config.cfg";
CConfig::TPtr gConfig;

CConfig::CConfig() {
    Load_Default_Path();
}

CConfig::~CConfig() {
}

void CConfig::Load_Default_Path() {
    std::ifstream file(Config_Filename);
    if (!file.is_open()) {
        std::cerr << "[[InTCPtor: could not open config file, using defaults]]" << std::endl;
        Save_Default();
        Initialize_Runtime();
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) {
            continue;
        }

        if (key == "Send__1B_Sends") {
            iss >> mProb_Send__1B_Sends;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config probability Send__1B_Sends = " << mProb_Send__1B_Sends << " ]]" << std::endl;
            }
        } else if (key == "Send__2B_Sends") {
            iss >> mProb_Send__2B_Sends;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config probability Send__2B_Sends = " << mProb_Send__2B_Sends << " ]]" << std::endl;
            }
        } else if (key == "Send__2_Separate_Sends") {
            iss >> mProb_Send__2_Separate_Sends;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config probability Send__2_Separate_Sends = " << mProb_Send__2_Separate_Sends << " ]]" << std::endl;
            }
        } else if (key == "Send__2B_Sends_And_Second_Send") {
            iss >> mProb_Send__2B_Sends_And_Second_Send;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config probability Send__2B_Sends_And_Second_Send = " << mProb_Send__2B_Sends_And_Second_Send << " ]]" << std::endl;
            }
        } else if (key == "Recv__1B_Less") {
            iss >> mProb_Recv__1B_Less;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config probability Recv__1B_Less = " << mProb_Recv__1B_Less << " ]]" << std::endl;
            }
        } else if (key == "Recv__2B_Less") {
            iss >> mProb_Recv__2B_Less;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config probability Recv__2B_Less = " << mProb_Recv__2B_Less << " ]]" << std::endl;
            }
        } else if (key == "Recv__Half") {
            iss >> mProb_Recv__Half;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config probability Recv__Half = " << mProb_Recv__Half << " ]]" << std::endl;
            }
        } else if (key == "Recv__2B") {
            iss >> mProb_Recv__2B;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config probability Recv__2B = " << mProb_Recv__2B << " ]]" << std::endl;
            }
        } else if (key == "Send_Delay_Ms_Mean") {
            iss >> mSend_Delay_Ms_Mean;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config Send_Delay_Ms_Mean = " << mSend_Delay_Ms_Mean << " ]]" << std::endl;
            }
        } else if (key == "Send_Delay_Ms_Sigma") {
            iss >> mSend_Delay_Ms_Sigma;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config Send_Delay_Ms_Sigma = " << mSend_Delay_Ms_Sigma << " ]]" << std::endl;
            }
        } else if (key == "Drop_Connections") {
            iss >> mDrop_Connections;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config Drop_Connections = " << mDrop_Connections << " ]]" << std::endl;
            }
        } else if (key == "Drop_Connection_Delay_Ms_Min") {
            iss >> mDrop_Connection_Delay_Ms_Min;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config Drop_Connection_Delay_Ms_Min = " << mDrop_Connection_Delay_Ms_Min << " ]]" << std::endl;
            }
        } else if (key == "Drop_Connection_Delay_Ms_Max") {
            iss >> mDrop_Connection_Delay_Ms_Max;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config Drop_Connection_Delay_Ms_Max = " << mDrop_Connection_Delay_Ms_Max << " ]]" << std::endl;
            }
        } else if (key == "Log_Enabled") {
            iss >> mLog_Enabled;
            if constexpr (Debug_Config_Outputs) {
                std::cout << "[[InTCPtor: config Log_Enabled = " << mLog_Enabled << " ]]" << std::endl;
            }
        }
    }

    Initialize_Runtime();
}

void CConfig::Save_Default() {
    // save the config file with values currently stored in the object
    std::ofstream file(Config_Filename);
    if (!file.is_open()) {
        std::cerr << "Error: could not open config file" << std::endl;
        return;
    }

    file << "Send__1B_Sends " << mProb_Send__1B_Sends << std::endl;
    file << "Send__2B_Sends " << mProb_Send__2B_Sends << std::endl;
    file << "Send__2_Separate_Sends " << mProb_Send__2_Separate_Sends << std::endl;
    file << "Send__2B_Sends_And_Second_Send " << mProb_Send__2B_Sends_And_Second_Send << std::endl;
    file << "Recv__1B_Less " << mProb_Recv__1B_Less << std::endl;
    file << "Recv__2B_Less " << mProb_Recv__2B_Less << std::endl;
    file << "Recv__Half " << mProb_Recv__Half << std::endl;
    file << "Recv__2B " << mProb_Recv__2B << std::endl;
    file << "Send_Delay_Ms_Mean " << mSend_Delay_Ms_Mean << std::endl;
    file << "Send_Delay_Ms_Sigma " << mSend_Delay_Ms_Sigma << std::endl;
    file << "Drop_Connections " << mDrop_Connections << std::endl;
    file << "Drop_Connection_Delay_Ms_Min " << mDrop_Connection_Delay_Ms_Min << std::endl;
    file << "Drop_Connection_Delay_Ms_Max " << mDrop_Connection_Delay_Ms_Max << std::endl;
    file << "Log_Enabled " << mLog_Enabled << std::endl;

    std::cout << "[[InTCPtor: saved config file]]" << std::endl;
}

void CConfig::Initialize_Runtime() {
    mRandEng.seed(std::random_device()());
    mSendDelayDist = std::normal_distribution<double>(mSend_Delay_Ms_Mean, mSend_Delay_Ms_Sigma);
    mProbDist = std::uniform_real_distribution<double>(0.0, 1.0);
}
