// PROVIDE DUMMY STUBS FOR FUNCTIONS THAT ARE NOT NEEDED
// WHEN THE UI IS NOT PRESENT (CONSOLE APPLICATIONS LIKE THE COMPILER)

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
