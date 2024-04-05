## Real-time SDR for mono/stereo FM and RDS

The main objective of the project is to navigate a complex speciﬁcation and develop an understanding of the challenges that must be addressed for a real-time implementation of a computing system operating in a form factor-constrained environment. 

The project description is available in the project [document](doc/3dy4-project-2024.pdf). The unique constraints for each group for different modes of operation (i.e., custom sample rates) are available [here](doc/3dy4-constraints-group-0.pdf).

All the project source code must be submitted before 7 p.m. on March 28. The project cross-examinations and oral presentations will run in the week of April 1, and the detailed final project report is due before 7 p.m. on April 5. 

Hello Grading Team,

In this Repo, we have our compiled effort for our SDR. 

We have accomplished the following sections:

MONO: All modes complete in src commit: b604716ac62599936c921642c1f303fde7882f11

STEREO: All modes complete in src commit:  b604716ac62599936c921642c1f303fde7882f11

When testing MONO and STEREO please checkout b604716ac62599936c921642c1f303fde7882f11 with commit message "mono fully implemented all modes stereo fully implemented all modes threading implemented rds implemented (does not work and partly turned off) rds works in model."

RDS: MODE 0 Works to acquire PTY and PI consistently, Mode 2 is far less consistent
program service is far less consistent in both modes.

When testing RDS the latest commit is bussin'

In Python, RDS has been modelled completely (given a strong signal, like samples3 or samples8) in OurRDS.py
We used some algorithms from PySDR for frame sync, and scipy libraries to supplement or signal processing

![image](https://github.com/3dy4-2024/project-group00-tuesday/assets/96309798/caef31c4-64ec-4028-9908-bebae00bd8b6)
