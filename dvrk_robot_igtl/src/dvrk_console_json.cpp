/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Anton Deguet
  Created on: 2015-07-18

  (C) Copyright 2015 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

// system
#include <iostream>
#include <map>

// cisst/saw
#include <cisstCommon/cmnPath.h>
#include <cisstCommon/cmnCommandLineOptions.h>
#include <sawIntuitiveResearchKit/mtsIntuitiveResearchKitConsole.h>
#include <sawIntuitiveResearchKit/mtsIntuitiveResearchKitConsoleQt.h>
#include <sawOpenIGTLink/mtsOpenIGTLinkBridge.h>

#include <QApplication>

#include <ros/ros.h>
#include <cisst_ros_bridge/mtsROSBridge.h>
#include <dvrk_utilities/dvrk_console.h>

void fileExists(const std::string & description, const std::string & filename)
{
    if (!cmnPath::Exists(filename)) {
        std::cerr << "File not found for " << description
                  << ": " << filename << std::endl;
        exit(-1);
    } else {
        std::cout << "File found for " << description
                  << ": " << filename << std::endl;
    }
}

int main(int argc, char ** argv)
{
    // log configuration
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskClass("mtsIntuitiveResearchKit", CMN_LOG_ALLOW_ALL);
    cmnLogger::AddChannel(std::cerr, CMN_LOG_ALLOW_ERRORS_AND_WARNINGS);

    // ---- WARNING: hack to remove ros args ----
    ros::V_string argout;
    ros::removeROSArgs(argc, argv, argout);
    argc = argout.size();
    // ------------------------------------------

    const double rosPeriod = 10.0 * cmn_ms;

    // parse options
    cmnCommandLineOptions options;
    std::string jsonMainConfigFile;
    std::string jsonCollectionConfigFile;
    std::string jsonIGTLConfigFile;
    std::string versionString = "v1_4_0";

    options.AddOptionOneValue("j", "json-config",
                              "json configuration file",
                              cmnCommandLineOptions::REQUIRED_OPTION, &jsonMainConfigFile);

    options.AddOptionOneValue("c", "collection-config",
                              "json configuration file for data collection",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &jsonCollectionConfigFile);

    options.AddOptionOneValue("o", "openigtlink-config",
                              "json configuration file for sawOpenIGTLink bridge",
                              cmnCommandLineOptions::REQUIRED_OPTION, &jsonIGTLConfigFile);

    options.AddOptionOneValue("m", "compatibility-mode",
                              "compatibility mode, e.g. \"v1_3_0\", \"v1_4_0\"",
                              cmnCommandLineOptions::OPTIONAL_OPTION, &versionString);

    // check that all required options have been provided
    std::string errorMessage;
    if (!options.Parse(argc, argv, errorMessage)) {
        std::cerr << "Error: " << errorMessage << std::endl;
        options.PrintUsage(std::cerr);
        return -1;
    }
    std::string arguments;
    options.PrintParsedArguments(arguments);
    std::cout << "Options provided:" << std::endl << arguments << std::endl;

    bool hasOpenIGTLink = options.IsSet("openigtlink-config");

    // make sure the json config file exists and can be parsed
    fileExists("JSON configuration", jsonMainConfigFile);
    if (hasOpenIGTLink) {
        if (!jsonIGTLConfigFile.empty()) {
            fileExists("OpenIGTLink configuration", jsonIGTLConfigFile);
        } 
    }

    // check version mode
    dvrk_topics_version::version versionEnum;
    try {
        versionEnum = dvrk_topics_version::versionFromString(versionString);
    } catch (std::exception e) {
        std::cerr << "Compatibility mode " << versionString << " is invalid" << std::endl;
        std::cerr << "Possible values are: ";
        std::cerr << cmnData<std::vector<std::string> >::HumanReadable(dvrk_topics_version::versionVectorString());
        std::cerr << std::endl;
        return -1;
    }
    std::cout << "Using compatibility mode: " << versionString << std::endl;

    mtsManagerLocal * componentManager = mtsManagerLocal::GetInstance();

    // console
    mtsIntuitiveResearchKitConsole * console = new mtsIntuitiveResearchKitConsole("console");
    console->Configure(jsonMainConfigFile);
    componentManager->AddComponent(console);
    console->Connect();

    // add all Qt widgets
    QApplication application(argc, argv);
    mtsIntuitiveResearchKitConsoleQt * consoleQt = new mtsIntuitiveResearchKitConsoleQt();
    consoleQt->Configure(console);
    consoleQt->Connect();

    // ros wrapper
    mtsROSBridge rosBridge("dVRKBridge", rosPeriod, true);
    dvrk::console * consoleROS = new dvrk::console(rosBridge, "/dvrk/",
                                                   console, versionEnum);
    componentManager->AddComponent(&rosBridge);
    consoleROS->Connect();

    // OpenIGTLink bridge
    mtsOpenIGTLinkBridge * igtlBridge;
    if (hasOpenIGTLink) {
        igtlBridge = new mtsOpenIGTLinkBridge ("bridge", 10.0 * cmn_ms);
        igtlBridge->Configure(jsonIGTLConfigFile);
        componentManager->AddComponent(igtlBridge);
        igtlBridge->Connect();
    }

    //-------------- create the components ------------------
    componentManager->CreateAllAndWait(2.0 * cmn_s);
    componentManager->StartAllAndWait(2.0 * cmn_s);

    application.exec();

    componentManager->KillAllAndWait(2.0 * cmn_s);
    componentManager->Cleanup();

    // stop all logs
    cmnLogger::Kill();

    delete consoleQt;

    return 0;
}
