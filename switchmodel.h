#pragma once
#include <iostream>
#include <vector>
#include "Mesh.h"
#include "Model.h"
#include<string>
using namespace std;


class More_models{
    public:
    vector<string> modelnames = {
    "koharu",
    "wakamo",
    };
    vector<string> Modelpaths = {
    "/home/kiyotaka/atcoder/QtOpengl/model/koharu/blue_archive_-koharu-__cb_default_emotion (1)/scene.gltf",
    "/home/kiyotaka/atcoder/QtOpengl/model/koharu/blue_archive_-wakamo_swimsuit-/scene.gltf",
    };
    int current_model_index = 0;
    Model* current_model = new Model(Modelpaths[current_model_index]);
    void SwitchModel(){
        if(current_model_index >= 0){
            delete current_model;
        }
        if(current_model_index == modelnames.size()-1){
            current_model_index = 0;
        }else{
            current_model_index += 1;
        }
        current_model = new Model(Modelpaths[current_model_index]);
    }
};

