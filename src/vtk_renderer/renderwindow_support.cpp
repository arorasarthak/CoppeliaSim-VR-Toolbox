// Copyright (c) 2018, Boris Bogaerts
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:

// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "renderwindow_support.h"
#include <vector>
#include "../remoteAPI/extApi.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <thread>
#include <chrono>


void ClickCallbackFunction(vtkObject* vtkNotUsed(caller), long unsigned int vtkNotUsed(eventId), void* in, void* vtkNotUsed(callData))
{
    renderwindow_support* temp = static_cast<renderwindow_support *>(in);
    temp->updatePose();
    temp->updateRender();
    temp->chrono->increment();
    if (temp->isReady()) {
        temp->syncData();
        temp->setNotReady();
    }
}



renderwindow_support::~renderwindow_support()
{

}


void renderwindow_support::updateRender() {
    vr_renderWindowInteractor->Render();
//    auto mat = vr_camera->GetCameraLightTransformMatrix();
//    std::cout << mat->GetElement(0,3) << ','
//    << mat->GetElement(1,3) << ','
//    << mat->GetElement(2,3) << '\n';
};

void renderwindow_support::updatePose() {
    vrepScene->updateMainCamObjectPose();
}

void renderwindow_support::syncData() {
    vrepScene->transferVisionSensorData();
}


void renderwindow_support::addVrepScene(vrep_scene_content *vrepSceneIn) {
    vrepScene = vrepSceneIn;
    vrepScene->vrep_get_object_pose();
    for (int i = 0; i < vrepScene->getNumActors(); i++) {
        vrepScene->getActor(i)->PickableOff();
        renderer->AddActor(vrepScene->getActor(i));
    }

    for (int i = 0; i < vrepScene->getNumRenders(); i++)
        renderer->AddActor(vrepScene->getPanelActor(i));

    if (vrepScene->isVolumePresent()) {
        renderer->AddViewProp(vrepScene->getVolume());
    }
    //renderer->ResetCamera();
    //vrepScene->getVolume()->Print(cout);
}

void renderwindow_support::visionSensorThread() {
    vrepScene->activateNewConnection(); // connect to vrep with a new port
    while (true) {
        vrepScene->updateVisionSensorObjectPose();
        vrepScene->updateVisionSensorRender();
        dataReady = true;
        while (dataReady) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        };
    }
}

void handleFunc(renderwindow_support *sup) {
    cout << "Vision sensor thread activated" << endl;
    sup->visionSensorThread();
}

void renderwindow_support::activate_interactor() {
//    // add actors
//
//    auto shadows = vtkSmartPointer<vtkShadowMapPass>::New();
//
//    auto seq = vtkSmartPointer<vtkSequencePass>::New();
//
//    auto passes = vtkSmartPointer<vtkRenderPassCollection>::New();
//    passes->AddItem(shadows->GetShadowMapBakerPass());
//    passes->AddItem(shadows);
//    seq->SetPasses(passes);
//
//    auto cameraP = vtkSmartPointer<vtkCameraPass>::New();
//    cameraP->SetDelegatePass(seq);
//
//    // tell the renderer to use our render pass pipeline
//    vtkOpenGLRenderer *glrenderer = dynamic_cast<vtkOpenGLRenderer*>(renderer.GetPointer());
//    glrenderer->SetPass(cameraP);
//    // lighting the box.

//    for (int i = 0; i < vrepScene->getNumberOfLights(); i++) {
//        auto l = vrepScene->getLight(i);
//        renderer->AddLight(l);
//        l->SetSwitch(1);
//    }

    vrepScene->vrep_get_object_pose();

    vr_camera->SetViewAngle(90.0);
    vr_camera->SetModelTransformMatrix(pose->GetMatrix());
    vr_camera->SetPosition(1.0,1.0,0.5);
    vr_camera->SetFocalPoint(-1,0.5,0.5);
    vr_camera->SetViewUp(0, 1, 0);


    renderer->SetBackground(1.0, 1.0, 1.0);
    renderer->SetActiveCamera(vr_camera);
    renderer->UseFXAAOn();

    double range[]  = { 0.01,100 };
    vr_camera->SetClippingRange(range);


    // Do some aestetic thingies
    simxFloat *data;
    simxInt dataLength;
    simxCallScriptFunction(clientID, (simxChar*)"HTC_VIVE", sim_scripttype_childscript,
            (simxChar*)"getEstetics", 0, NULL, 0, NULL, 0,
            NULL, 0, NULL, NULL, NULL, &dataLength, &data,
            NULL, NULL, NULL, NULL, simx_opmode_blocking);

    if (dataLength > 0) {
        renderer->SetBackground(data[0], data[1], data[2]);
        renderer->SetBackground2(data[3], data[4], data[5]);
        renderer->SetGradientBackground(true);
    }
    renderer->SetAmbient(1.0, 1.0, 1.0);

    renderer->LightFollowCameraOn();
    renderer->AutomaticLightCreationOn();
    //renderer->SetAutomaticLightCreation(false);
    //renderer->UseShadowsOn();

    renderer->Modified();

    renderWindow->AddRenderer(renderer);
    renderWindow->SetMultiSamples(0);
    renderWindow->SetDesiredUpdateRate(90.0);
    renderWindow->SetSize(1920, 1080);
    renderWindow->PolygonSmoothingOn();


    vr_renderWindowInteractor->SetRenderWindow(renderWindow);

    vrepScene->vrep_get_object_pose();

    vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
    vr_renderWindowInteractor->SetInteractorStyle(style);
    vr_renderWindowInteractor->Initialize();
    // Callback

    vtkSmartPointer<vtkCallbackCommand> clickCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    clickCallback->SetCallback(ClickCallbackFunction);
    clickCallback->SetClientData(this);

    vr_renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, clickCallback);
    vr_renderWindowInteractor->CreateRepeatingTimer(5);

    std::thread camThread(handleFunc, this);

    //this->updatePose();
    vr_renderWindowInteractor->Start();
}

