Technical Overview {#Technical_Overview}
==================

[TOC]

# Technical Overview {#Technical_Overview}

The Interactive Supercomputing library helps users to construct loosely
coupled simulation-observer/controller pairs in an easy manner. After the
integration of all the components, users can interact with a simulation both
for streaming events from it and steering any of its parameters.

To accomplish this task, Monsteer provides: Brion plugins for spike data streaming,
a Python interface to steer the simulator and a bridging application between
the user and the simulator.

At the moment, Monsteer only offers support for the \ref NEST_Simulator which is
mainly used by the Neurobotics group in the BlueBrain project. Despite this
limitation, all the Monsteer components were developed thinking on their
reusability with other simulators such as Neuron.

For example, Monsteer is based on the plugin mechanism provided by Brion, where the
interface of spike streaming is defined. By using different schemas,
developers have the ability of implementing different spike
readers/writers. These are all interfaced through the BBPSDK API, where the
exposed interface is common for all the spike stream processes. As a result,
for users it is transparent to use different spike sources where they only
have to provide different spike readers or writers. These mechanisms are
already used to stream and read file data from NEST and Neuron simulators by
just using different schemas.

A general overview of the Monsteer project can be seen below.

![](general_overview.png "General Overview")

# NEST Neural Network Simulator {#NEST_Simulator}

NEST is a large-scale neural network simulator that can be run on distributed
computing environments. The data flow in the distributed running environment is
mainly based on the MPI library.  Different NEST processes can either share data
either by just using MPI, or through the \ref Music_Library, a library that
facilitates the unification of different simulators using a common interface.

Monsteer uses the MUSIC interface integrated in the NEST simulator for steering and
streaming. The functionalities are exposed in the Python interface of the
simulator. This interface allows the Python classes, namely \ref Nesteer, to be
developed in order to provide support for sending and receiving steering
messages and spike events.

# MUSIC Library {#Music_Library}

MUSIC is a well-known data exchange library for neural simulators such as
NEST, Neuron and Moose.  It can connect large scale neural networks and
has an easy interface for coupling applications with the simulators.

Using static configurations between applications which are running at
different speeds, the MUSIC library is capable of synchronizing data by
employing internal buffering and synchronization mechanisms. Developers using
MUSIC to build such applications are abstracted from this synchronization
mechanisms, making the development process easier rather than doing it by
low-level MPI calls.

MUSIC is used in the Music Proxy application to receive spikes from a NEST
simulator and to communicate the steering messages to and from the
simulator. The underlying mechanisms in the library helped the proxy
application to be developed in an very flexible way, also making it easily
extensible to make the coupling with other libraries.

# Brion Library {#Brion_Library}

Brion provides many additional plugins and plugin interfaces for accessing
BlueBrain project data. This data includes neuron structures, neuron
placements, simulation results, meshes, etc. The plugins in this library are
activated by a schema, where a schema can also be a file extension rather than
a fully qualified URL.

Monsteer extends the spike reader and writer interfaces defined in the Brion plugin
API. When a library is linked against the Monsteer library, those plugins are
automatically instantiated. Here Monsteer library provides schema starting with
"Monsteer" keyword, such as "monsteer://" for spike streaming and "MONSTEER_steer://" for
steering.

# Brion Spikes {#Brion_Spikes}

Spike events are one of the main simulation results where visualization and
analysis is important for neuroscientists. A spike event is a binary event
which takes place in a neuron on a certain time within the simulation, when
neuron passes a certain voltage threshold. This spike event can be represented
as a timestamp and a neuron identifier. In Brion and Monsteer, there is support
for internal BBP data formats, NEST file formats and streaming formats.

# Zeq ZeroEq: Auto-dIscovery and Event Based Data Sharing Library

The ZeroEQ library is used for auto-dIscovery of services, data transport and
event based messaging. The auto-dIscovery and data transport are done
through the ZeroMQ library, where in lack of auto-dIscovery, it is also possible
to target a host with a given port. The messages are based on FLATBuffers schema
which enforces the messages to be well-defined.  On top of these libraries,
ZeroEQ provides an URI-based auto-dIscovery, event listening and event
registration layers. In ZeroEQ the events form a vocabulary for a given schema
and can easily be extended.

Two ends of the connection in ZeroEQ is a publisher and subscriber pair. When
auto-dIscovery is possible, a publisher announces its schema to the outer world
in the network and subscribers interested with this schema can connect to this
publisher.

When auto-dIscovery is not available or is not requested by the subscriber, it
is also possible to provide a specific host and port in the schema. This way,
a connection to a known publisher can be established.

In Monsteer, ZeroEQ is used for: sending spike events from Music Proxy to the
steering application, sending steering messages from the steering application
and sending information messages from the simulator. Each item introduces a
schema, its own vocabulary and consequently its own events.

# Music Proxy {#Music_Proxy}

The Music Proxy acts as a bridge between the simulation and the user
application, making use of MUSIC communicate steering messages between both
components, as well as receiving spike events from the simulation in order to
send them later to the user application.

The application is composed of two main components: the MUSIC spike event
handler and the MUSIC steering handler. Within the application, these
components are synchronized on the same loop to send and receive data.

A general overview of the Music Proxy and its software components can be seen
below.

![](detailed_music_proxy.png "Music Proxy")

## Spike Event Handler {#Music_Proxy_Spike_Event_Handler}

The spike event handler is responsible for receiving the spike events and
forwarding them to the application using the ZeroEQ library. MUSIC has
mechanisms to register for spike events and using the synchronization
mechanisms available in the library, these events are captured and translated
into the spike data format used in Brion.

## Spike Streaming {#Spike_Streaming}

The spike event reader and writer interfaces are declared in Brion. The
necessary structures for reading/writing files and streaming spikes are
developed within the Brion and Monsteer libraries.

Monsteer provides the spike streaming plugins. Based on the ZeroEQ library, the
spikes are transferred from one end to another. Both ends, publishers and
subscribers, connect through a given URL schema.  In the case of spike
streaming, the URL schema is "monsteer://". If auto-dIscovery is enabled giving
this schema is sufficient for both ends to connect, otherwise it is also
possible to hardcode the host and
its port for the connection.

## Steering Handler {#Music_Proxy_Steering_Handler}

The steering handling is based on two main components. The first component
forwards steering messages from the steering application to the NEST
simulation. The receipt of steering messages is handled through the Zeq
library and the forwarded messages are sent through the MUSIC library.

The second component receives messages from the NEST simulation and forwards
them to the steering application. Messages are received from the NEST
simulation through the MUSIC library, and sent to the user application through
ZeroEQ.

# BBPSDK Simulation Controller {#BBPSDK_Simulation}

BBPSDK is a basic library for accessing the BlueBrain data. It can read spike
reports, compartment reports, meshes, neuron structures, their placements,
etc. It provides a complete user API and their Python wrappings where the
implementations are generally plugins. The right plugin is selected based on a
given schema. These plugins are generally located in Brion library and can
easily be extended by 3rd party libraries. Monsteer library is one of the libraries
that provides plugins for the Monsteer schema.

For users to steer a simulation easily, a new functionality was added in the
BBPSDK library. The functionality is abstracted from the underlying simulator
using the plugin mechanism. At the moment there is only one plugin in Monsteer for
steering NEST simulations, but for the future new functionalities can be added
for different simulators such as Neuron. With the simulator interface, a user
can inject stimuli into a running simulation, play and pause the simulation
and receive additional information from it.

The functionality added in BBPSDK was also wrapped in Python to make it more
useful for applications that have a Python interface i.e: RTNeuron. Later,
the Python wrapping will be extended for the NEST use case in monsteer.

A general overview of the BBPSDK Simulation Controller and its software
components can be seen below.

![](detailed_steering_streaming.png "Controlling the simulation")

# Nesteer: A Python interface for processing the steering messages {#Nesteer}

NEST users generally use a Python shell for constructing and running their
simulations. If at some point they want to add steering and streaming
capabilities to their simulation, Monsteer can facilitate this process by providing
a Python object to process the received messages. Using the provided Python
object, a user can easily define what to do when a steering message is
received by providing new functions for certain events.

Nesteer provides internal mechanisms to setup the MUSIC communication channels
and connect the neurons. For its use with BBP data, it provides a mapping
between NEST neuron ids and BBP circuit neuron ids. That way, users can
transparently work with BBP circuits.

![](detailed_nest_simulation.png "Nesteer and NEST")


