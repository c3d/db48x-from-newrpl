// ****************************************************************************
//  non-gui-stubs.c                                               DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Stubs for functions that are not needed when the UI is not present
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms in LICENSE.txt
// ****************************************************************************

void thread_yield()
{
}

void thread_processevents()
{

}

void stop_singleshot()
{

}

void timer_singleshot(int ms)
{
    (void)ms;
}
