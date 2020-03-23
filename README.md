# Vulkan-test
A test of the Vulkan api

>Results: Vulkan does give much more freedom in memory and the debugging is better however, portability is more
difficult. With OpenGL I was able to build on my work machine and copy the .exe to a different windows machine and it would run.
With vulkan there are issues of the library(vulkan.dll) being on the machine but also there is introduced complexity when it comes to 
checking for available GPU's with available queues, enough memory(or more work in writing fallbacks) and even features such as multisampling and
window system integration.

I believe the couple of milliseconds per frame I would get out of Vulkan do not outway the extra months of work in ensuring 
compatibility amongst many different devices.
