// Make sure to have the server side running in CoppeliaSim:
// in a child script of a CoppeliaSim scene, add following command
// to be executed just once, at simulation start:
//
// simRemoteApi.start(19999)
//
// then start simulation, and run this program.
//
// IMPORTANT: for each successful call to simxStart, there
// should be a corresponding call to simxFinish at the end!

extern "C" {
#include "remoteAPI/extApi.h"
}
#include <iostream>
#include "vtk_renderer/vrep_scene_content.h"
#include "vtk_renderer/renderwindow_support.h"

using namespace std;

int main(int argc,char* argv[])
{
    //	 First make a connection to vrep
    int portNb = 19999;
    int clientID = -1;
    int clientID2 = -1;
    int running = 0;

    cout << "Trying to connect with V-REP ..." << endl;
    while (clientID == -1) {
        extApi_sleepMs(100);

        clientID = simxStart((simxChar*)"127.0.0.1", portNb, true, true, -200000, 5);
    }

    cout << endl << "Connected to V-REP, clientID number : " << clientID << endl;
    cout << "Connected to V-REP port number : " << portNb << endl << endl;

    simxInt *data;
    simxInt dataLength;
    simxInt result;
    int interactor;
    while (true) {
        // Wait until simulation gets activated
        cout << "Wait for simulation start ..." << endl;
        result = 8;
        while ((result == 8) || (running == 0)) {
            result = simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript, (simxChar*)"isRunning"
                    , 0, NULL, 0, NULL, 0, NULL, 0, NULL, &dataLength, &data, NULL, NULL, NULL, NULL, NULL, NULL, simx_opmode_blocking);
            if (result != 8) {
                running = data[0];
                interactor = data[1];
            }
        }
        cout << "Simulation started" << endl << endl;
        int refHandle;

        simxGetObjectHandle(clientID, (simxChar*)"HTC_VIVE", &refHandle, simx_opmode_blocking);
        vrep_scene_content *scene = new vrep_scene_content(clientID, refHandle); // all geometry is relative to refference frame
        scene->loadVolume();
        scene->loadScene(true); // read scene from VREP
        scene->loadCams();
        scene->connectCamsToVolume();
        scene->checkMeasurementObject(); // if the volume is replaced by a mesh then we need to change a few things
        renderwindow_support *supp = new renderwindow_support(clientID);
        supp->addVrepScene(scene);
        supp->activate_interactor();
        delete supp;
        delete scene;
        extApi_sleepMs(500);
    }
    return(0);
}
