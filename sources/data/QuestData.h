//
// Created by bison on 10-06-25.
//

#ifndef SANDBOX_QUESTDATA_H
#define SANDBOX_QUESTDATA_H

enum class QuestStatus { NotStarted, Active, Completed, Failed };

struct QuestSaveState {
    QuestStatus status;
    int stage; // optional: which part of the quest you're on
};

#endif //SANDBOX_QUESTDATA_H
