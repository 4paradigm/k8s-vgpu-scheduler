# Design

!<img src="../imgs/arch.png" width = "800" /> 

The architect of HAMi is shown in the figure above, It is organized in the form of "chart".

- MutatingWebhook

The MutatingWebhook checks the validity of each task, and set the "schedulerName" to "HAMi scheduler" if the resource requests have been recognized by HAMi
If Not, the MutatingWebhook does nothing and pass this task to default-scheduler.

- Scheduler

- DevicePlugin

- InContainer Control
