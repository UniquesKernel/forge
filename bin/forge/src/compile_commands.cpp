#include "compile_commands.hpp"
#include "assert.hpp"
#include "constants.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <vector>

std::vector<CompileTask> build_compile_commands_json(forge::parser::Settings*    settings,
                                                     const std::filesystem::path path) {
        std::vector<CompileTask> tasks = std::vector<CompileTask>();

        std::filesystem::create_directories(OBJECT_FOLDER);

        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_directory() || entry.path().extension() != SOURCE_FILE_EXTENSION) {
                        continue;
                }

                const std::filesystem::path src      = entry.path();
                const std::filesystem::path obj_dir  = std::filesystem::path(OBJECT_FOLDER) / src.parent_path();
                std::filesystem::path       obj_file = (obj_dir / src.stem()).string() + OBJECT_FILE_EXTENSION;

                std::filesystem::create_directories(obj_dir);

                char** argv = (char**)(malloc(10 * sizeof(char*)));
                argv[0]     = strdup(settings->config.compiler.c_str());
                argv[1]     = strdup(src.c_str());
                argv[2]     = strdup(settings->config.compiler_standard.c_str());
                argv[3]     = strdup("-g");
                argv[4]     = strdup("-Iinclude");
                argv[5]     = strdup("-c");
                argv[6]     = strdup("-o");
                argv[7]     = strdup(obj_file.c_str());
                argv[8]     = nullptr;

                CompileTask task;
                task.argv           = argv;
                task.file           = strdup(src.c_str());

                std::string cmd_str = settings->config.compiler;
                cmd_str += " " + src.string() + " " + settings->config.compiler_standard + " -g -Iinclude -c -o " +
                           obj_file.string();
                task.cmd_str = strdup(cmd_str.c_str());

                tasks.push_back(task);
        }

        unsigned int max_jobs = std::thread::hardware_concurrency();
        if (max_jobs == 0)
                max_jobs = 2; // Safety fallback

        printf("Forge: Parallel build initialized with %u max jobs.\n", max_jobs);

        std::vector<pid_t> active_pids;
        size_t             task_idx     = 0;
        bool               build_failed = false;

        while (task_idx < tasks.size() || !active_pids.empty()) {
                // 2. Spawn phase: Fill available slots
                while (active_pids.size() < max_jobs && task_idx < tasks.size()) {
                        if (build_failed)
                                break; // Stop spawning new work if something already broke

                        pid_t pid = fork();
                        INVARIANT(pid >= 0);

                        if (pid == 0) {
                                // Child Process
                                execv(tasks[task_idx].argv[0], tasks[task_idx].argv);
                                perror("execv failed");
                                exit(1);
                        } else {
                                // Parent Process
                                active_pids.push_back(pid);
                                printf("[SPAWN] Started task %zu/%zu (PID: %d) | Active Slots: %zu/%u\n", task_idx + 1,
                                       tasks.size(), pid, active_pids.size(), max_jobs);
                                task_idx++;
                        }
                }

                // 3. Wait phase: Reclaim slots as they finish
                if (!active_pids.empty()) {
                        int   status;
                        // wait(-1, ...) waits for ANY child process to finish
                        pid_t finished_pid = wait(&status);

                        if (finished_pid > 0) {
                                // Find and remove the PID from our active tracking list
                                auto it = std::find(active_pids.begin(), active_pids.end(), finished_pid);
                                if (it != active_pids.end()) {
                                        active_pids.erase(it);
                                }

                                if (WIFEXITED(status)) {
                                        int code = WEXITSTATUS(status);
                                        if (code == 0) {
                                                printf("[DONE] PID %d finished successfully. Slots remaining: %zu\n",
                                                       finished_pid, active_pids.size());
                                        } else {
                                                printf("[ERROR] PID %d failed with code %d. Stopping further spawns.\n",
                                                       finished_pid, code);
                                                build_failed = true;
                                        }
                                }
                        }
                }
        }

        if (build_failed) {
                printf("Build aborted due to previous errors.\n");
                return {}; // Logic to stop the linker would go here
        }

        std::ofstream json_out(COMPILE_COMMANDS_FILE);
        json_out << "[\n";
        for (size_t i = 0; i < tasks.size(); ++i) {
                json_out << "  {\n"
                         << "    \"directory\": \"" << std::filesystem::current_path().string() << "\",\n"
                         << "    \"command\": \"" << tasks[i].cmd_str << "\",\n"
                         << "    \"file\": \"" << tasks[i].file << "\"\n"
                         << "  }" << (i == tasks.size() - 1 ? "" : ",") << "\n";
        }
        json_out << "]";

        return tasks;
}
