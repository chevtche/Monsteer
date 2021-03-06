User Guide {#User_Guide}
==========

[TOC]

# Overview {#Overview}

The Monsteer project is a set of libraries and tools that helps users to visualize,
analyse and steer their simulations. Currently, the target community are NEST
users who write simulators using the PyNEST interface. These users generally
want to visualize the neural activity and and possibly steer the simulation
accordingly. The Monsteer library fills this gap, providing the necessary mechanisms
to stream and steer the simulation results through user-friendly python modules.

# Compilation {#Compilation}

Minimum configuration to configure using cmake, compile and run Monsteer:

* A Linux box,
* GCC compiler 4.8+,
* CMake 2.8+,
* Boost 1.54,
* MPI (OpenMPI, mvapich2, etc),
* NEST simulator 2.4.2,
* MUSIC 1.0.7,
* Python 2.6,

# Setup {#Setup}

After the Monsteer project is compiled and installed, an executable called
*music_proxy* and the Python packages *bbp* and *Monsteer* will be present in the
installation directory.

Before running music configurations, the environmental variables PATH and
PYTHONPATH must be configured to point to the location of the music_proxy binary
and the Python packages *bbp* and *Monsteer* respectively. To visualize spike
activity, the RTNeuron application has to be be installed as well.

Sample scripts and configuration files for steering and streaming are provided
in the *share/monsteer/examples/nest* directory from the installation tree.

## Working in BBP Infrastructure {#BBP_Infra}

All the visualization tools, including Monsteer, are regularly made available on the
40 node visualization cluster in Lugano, at their latest version by the
visualization team. Therefore, it is highly recommended for the users to work
with this infrastructure.

After allocation of resources in this cluster, all tools become usable with the
command line below:

~~~
bbpviz023~ % module load BBP/viz/latest
~~~

## Music Configuration {#Music_Configuration}

In the share/monsteer/examples/nest directory there are files with the extension
".music" and there are NEST simulation scripts. MUSIC configuration describes
how to launch multiple applications that will communicate using MUSIC, how many
MPI processes a MUSIC application will use and what ports should be connected.

Below there is a sample music configuration:

~~~
# Usage:
# - With mpirun, form this directory do:
#   $ mpirun -np 5 music nest2music_proxy_with_steering.music
# - With srun
#   srun -n 5 music nest2music_proxy_with_steering.music

stoptime=10000000

[mars]
  binary=./minimal_steered.py
  np=4

[venus]
  # This binary is expected to be in the path
  binary=music_proxy
  args=--steering
  np=1

mars.spikes_out -> venus.spikesPort [1000]
venus.steeringPort -> mars.steering_input [0]
~~~

In this configuration:

Sections:
* "stoptime" is the total simulation time given in seconds,
* "binary" is an executable name,
* "np" is the number of MPI processes,
* "args" contains the command line arguments.

Options:
* "mars" and "venus" are application definitions which will be executed using MUSIC environment,

Connection definitions:
* "mars.spikes_out -> venus.spikesPort [1000]" constructs the "spike event"
  connection from the "mars" application to the "venus" application. "1000" is
  the number of channels this connection has. In the Monsteer case, there are 1000
  neurons that reports their spike activity.
* "venus.steeringPort -> mars.steering_input [0]" connects "messaging"
  ports. "0" means, there will be only one channel to communicate. This
  parameter is used for setting up the messaging ports.

Please refer to the [MUSIC](http://incf.org/activities/our-programs/modeling/music)
documentation for more detailed documentation.

## NEST script {#Nest_script}

The NEST script is responsible for running the neural simulation. For the NEST
simulator to communicate to "music_proxy" application, the spike events and
steering MUSIC ports should be connected. The script below setups the MUSIC
communication ports and runs the simulation.

\code{.py}
duration = 100000
cell_count = 1000

import nest

neurons = Create("iaf_neuron", cell_count)

# Setups music spike output.
music_spike_output = Create('music_event_out_proxy', 1)
SetStatus(music_spike_output, {'port_name': 'spikes_out'})

# Connecting neurons to music event channels.
for i in range(0, len(neurons)):
    Connect([neurons[i]], music_spike_output, 'one_to_one',
                  {'music_channel': i+1})

# Setup music steering input.
music_steering_input = _nest.Create('music_message_in_proxy', 1)
SetStatus(music_steering_input,
                {'port_name': 'steering_input',
                 "acceptable_latency": 40.0})

time = 0
while time < duration:
    Simulate(10)
    time += 10
\endcode

The above code lacks parsing the steering messages. Monsteer adds the @ref
Nesteer python module, to simplify the process of steering.

## Nesteer {#Nesteer}

The Nesteer python class helps users to setup MUSIC communication and map BBP
circuits to NEST simulations. The example below simplifies the previous NEST
script by adding the Nesteer steering message parser.

\code{.py}
duration = 100000 #In miliseconds.
cell_count = 1000

from monsteer.Nesteer import Nesteer
from nest import *

neurons = Create("iaf_neuron", cell_count)

monsteer_communicator = Nesteer('spikes_out',
                           'steering_input',
                           neurons,
                           [i + 1 for i in range(cell_count)])

def apply_spikes(generator, neurons):
    Connect(generator, neurons, 'all_to_all', syn_spec={'weight': 40.0})
monsteer_communicator.set_apply_generators_to_neurons_function(apply_spikes)

time = 0
while time < duration:
    Simulate(10)
    monsteer_communicator.process_events()
    time += 10
\endcode

The parameters of the Nesteer class:

- 'spikes_out' is MUSIC spike event port name,
- 'steering_input' is MUSIC messaging port name for steering,
- 'neurons' is the Nesteer neurons.
- '[i + 1 for i in range(cell_count)]' is which neurons corresponds to which
  channel in event port.  Here in this example event ports are connected for
  each of the 1000 neurons. And, they are each of them is mapped to each event
  channel from 1 to 1000. This corresponds to the MUSIC configuration event
  channel count.

In case of BBP circuit, it is the mapping of NEST neuron ids too BBP neuron
ids, because the NEST and BBP has different ids for cells. For to take BBP
circuit into account, the event channels in MUSIC configuration should be set
to maximum neuron id.

# Working with the Simulation {#Working_With_Simulation}

After having the configuration and script files, the simulation can be executed
using either mpirun or srun depending on the MPI environment setup. If there is
the MUSIC configuration file named "config.music", and the total number of
processes configured in this file is "5", then either it is run with mpirun or
srun as below.

~~~
srun -n 5 music config.music
~~~
or
~~~
mpirun -np 5 music config.music.
~~~

This will trigger running of the simulation and sharing information with the
"music_proxy" application.

## Steering {#Steering}

To send steering commands to the simulation, there is a python NEST helper
object. Through this object, injecting different spike generators to neurons,
playing and pausing the simulation is possible. In the below example,
injecting stimuli to multiple neurons and then pausing the simulation example
can be seen.

\code{.py}

import monsteer.Nesteer
import bbp
import nest
import numpy
import time

# Creating the steering connection
simulator = monsteer.Nesteer.Simulator('monsteer_nesteer://')

# Get parameter dictionary for a spike generator
params = nest.GetDefaults('poisson_generator')

# Modify rate
params['rate'] = 50000.0

# Define which cells will be effected by the spike generator
cells = bbp.Cell_Target()
for i in range(1000):
    cells.insert(i + 1)

time.sleep(4) # Waiting for connection

# Inject stimulus to the simulator
simulator.injectStimulus(params, cells)
time.sleep(4) # Waiting for connection

# Pause the simulator
simulator.pause()
\endcode

## Spike Visualization {#Spike_visualization}

To visualize the spikes of a BBP circuit, the RTNeuron circuit visualization
tool can be employed. It has the features to connect to a running simulation and
visualize spike events in the circuit.

In below example, a BlueBrain circuit is utilized. The configuration file for
the circuit can be accessed from the lugano cluster, from path
"/gpfs/bbp.cscs.ch/apps/viz/bbp/dev/BBPTestData/master/BBPTestData/Build/include/BlueConfig".

In this circuit there are 1000 neurons. The sample simulation scripts in the
source code repository in Monsteer can be employed to run simulations on them.

After running the simulation with MUSIC configuration, a user can launch
RTNeuron with below command line ( the --gui parameter activates the gui and
connection to a simulator can be run using the "OpenSimulation" button in the
gui ):

~~~
rtneuron-app.py -b "BLUE CONFIG FILE" --target Column soma --neurons 1 1000 soma --shell --gui [ -s monsteer:// ]
~~~

If the GUI is enabled, clicking on the "OpenSimulation" button brings a dialog
for entering the url for the music proxy spike port. If there is only one
simulation running, simply "monsteer://" can be entered, else "monsteer://host:port"
should be entered for the right host and port which provides the spikes. If
GUI is not enabled, "-s monsteer://" parameter can be added to the RTNeuron
execution command line.

Once the connection is established through GUI, spike events should be visible
on the screen.  Interacting with GUI item "spike tail" slider, changes how
long a spike is visible and "simulation delta" changes the the speed of
simulation.

If connection is established through command line parameter, soma additional
commands should be executed in python shell to start visualizing the spikes,
changing spike tails and changing the simulation delta. Those can be seen
below:

\code{.py}
app.player.play()
app.player.simulationDelta = 0.1
view.attributes.spike_tail = 0.01
\endcode

