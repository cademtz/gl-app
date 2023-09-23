/**
 * @file platform.hpp
 * @brief Define functions to interact with the platform
 *
 */

#pragma once
#include <functional>

namespace hid { class InputHandler; }

#define PLATFORM_WARNING(Msg) Platform::Warning(Msg, __FILE__, __LINE__)
#define PLATFORM_ERROR(Msg) Platform::Error(Msg, __FILE__, __LINE__)

namespace Platform {
using RepeatTaskCallback = std::function<bool()>;

void SetInputHandler(hid::InputHandler* handler);
/** Set the return value of @ref ShouldClose to `true` */
void SetShouldClose();
/** @return `true`, if the app should close gracefully */
bool ShouldClose();
/**
 * Prepare the application to run on its current platform.
 * Initialize any necessary APIs, windows, input callbacks, and more.
 */
void Setup();
/** Perform necessary cleanup (if any) before the application exits. */
void Cleanup();
/** Called by application before rendering */
void PreRender();
/** Called by application after rendering */
void PostRender();
/** Exit the application gracefully. Calls @ref Cleanup */
void Exit();
/**
 * @brief Run all tasks once, removing any finished ones.
 *  Tasks are provided by @ref AddRepeatingTask
 * 
 * @return `false`, if there are no tasks
 */
bool RunTasksOnce();
/**
 * @brief Block execution by running @ref RunTasksOnce until no tasks remain,
 *  or until @ref ShouldClose returns false.
 */
void RunTasksForever();
/**
 * @brief Add a task to be executed repeatedly by @ref RunTasksforever
 *
 * @param task A function that performs operations without any waiting,
 *  and returns `true` if it should be ran again.
 *  (Render a frame, read/write pending IO in a non-blocking manner, update loading progress, ...)
 */
void AddRepeatingTask(RepeatTaskCallback task);
/** Get the size of the main window's render buffer in pixels */
void GetFrameBufferSize(int* out_width, int* out_height);
void Warning(const char* Msg, const char* File = 0, int Line = -1);
void Error(const char* Msg, const char* File = 0, int Line = -1);

}