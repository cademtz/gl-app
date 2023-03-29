#include <platform.hpp>
#include <app.hpp>
#include <cstdio>

extern "C" int main(int argc, char** argv)
{
    Platform::Setup();

    App::OnSetup();

    Platform::AddRepeatingTask(
        [] {
            App::Loop();
            return true;
        }
    );
    
    Platform::RunTasksForever();

    Platform::Exit();
}