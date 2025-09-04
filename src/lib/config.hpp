/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains configuration settings for the library.
 */

#pragma once

#include <string>
#include <random>

#include <memory>

class CConfig {
    private:
        double mProb_Send__1B_Sends = 0.1;
        double mProb_Send__2B_Sends = 0.1;
        double mProb_Send__2_Separate_Sends = 0.3;
        double mProb_Send__2B_Sends_And_Second_Send = 0.2;

        double mProb_Recv__1B_Less = 0.1;
        double mProb_Recv__2B_Less = 0.1;
        double mProb_Recv__Half = 0.3;
        double mProb_Recv__2B = 0.2;

        double mSend_Delay_Ms_Mean = 100;
        double mSend_Delay_Ms_Sigma = 10;

        bool mDrop_Connections = false;
        size_t mDrop_Connection_Delay_Ms_Min = 5000;
        size_t mDrop_Connection_Delay_Ms_Max = 15000;

        std::default_random_engine mRandEng;
        std::normal_distribution<double> mSendDelayDist;
        std::uniform_real_distribution<double> mProbDist;

    protected:
        void Initialize_Runtime();

    public:
        using TPtr = std::unique_ptr<CConfig>;

        CConfig();
        virtual ~CConfig();

        void Load_Default_Path();
        void Save_Default();

        double GetProb_Send__1B_Sends() const { return mProb_Send__1B_Sends; }
        double GetProb_Send__2B_Sends() const { return mProb_Send__2B_Sends; }
        double GetProb_Send__2_Separate_Sends() const { return mProb_Send__2_Separate_Sends; }
        double GetProb_Send__2B_Sends_And_Second_Send() const { return mProb_Send__2B_Sends_And_Second_Send; }
        double GetProb_Send_Total() const { return mProb_Send__1B_Sends + mProb_Send__2B_Sends + mProb_Send__2_Separate_Sends + mProb_Send__2B_Sends_And_Second_Send; }

        double GetProb_Recv__1B_Less() const { return mProb_Recv__1B_Less; }
        double GetProb_Recv__2B_Less() const { return mProb_Recv__2B_Less; }
        double GetProb_Recv__Half() const { return mProb_Recv__Half; }
        double GetProb_Recv__2B() const { return mProb_Recv__2B; }
        double GetProb_Recv_Total() const { return mProb_Recv__1B_Less + mProb_Recv__2B_Less + mProb_Recv__Half + mProb_Recv__2B; }

        double Generate_Send_Delay() {
            return mSendDelayDist(mRandEng);
        }

        double Generate_Base_Prob() {
            return mProbDist(mRandEng);
        }

        bool Should_Drop_Connections() const { return mDrop_Connections; }
        size_t GetDrop_Connection_Delay_Ms_Min() const { return mDrop_Connection_Delay_Ms_Min; }
        size_t GetDrop_Connection_Delay_Ms_Max() const { return mDrop_Connection_Delay_Ms_Max; }
};

extern CConfig::TPtr gConfig;
