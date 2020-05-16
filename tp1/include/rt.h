//
// Created by tarsio on 12/05/2020.
//

#ifndef TASKER_RT_H
#define TASKER_RT_H

#endif //TASKER_RT_H
// start list
int rt_init();
// add f to list
int rt_insert(void (*fp)(), const char *name, int period, int deadline);
// rm f from list
int rt_insert(void (*fp)(), const char *name, int period, int deadline);
// start rt
int rt_start(void (*fp)(), const char *name, int period, int deadline);
// start rt
int rt_start(void (*fp)(), const char *name, int period, int deadline);
