#pragma once

class Range
{
public:
    Range() : start(0), end(0), stepSize(0) {};

public:
    int start;
    int end;
    int stepSize;

};

class CAbstractTask
{
public:
    virtual void RunTask() =0;
    virtual bool Split(CAbstractTask **newTask) =0;

};