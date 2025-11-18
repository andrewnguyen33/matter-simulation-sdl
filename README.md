# Matter Simulation
A C-based real-time fluid simulation engine built with SDL, modeling water, gas, and solid-block interactions using custom physics rules, cell-based pressure dynamics, and efficient per-frame state updates. Includes smooth vertical/horizontal flow behavior, pressure-driven movement, and dynamic rendering of liquid/gas levels with interpolated color gradients.

## How It's Made:

This project began as a water–solid cellular simulation and later expanded to include gas dynamics. The simulation is rendered in an SDL window divided into discrete cells, each represented by a custom Cell struct containing a material type, fill level, and grid coordinates. Water behavior is governed by three core rules:

Downward Flow – Water moves into the cell below when that cell has a lower fill level, simulating gravity-driven descent.

Sideways Flow – When downward flow is blocked by a solid or full cell, water redistributes left and right, equalizing fill levels unless obstructed.

Upward Pressure – If both downward and lateral movement are restricted and the current cell exceeds a full fill level (pressure buildup), excess water is pushed upward.

Gas dynamics are implemented with two complementary rules:

Upward Flow – Gas rises into cells with lower fill levels, simulating buoyancy.

Lateral Expansion – Gas spreads sideways as it moves upwards, distributing evenly across available space.

These rules are applied each frame through a series of simulation phases that update a copy of the environment before committing changes, ensuring stable and predictable behavior. The result is an efficient, real-time simulation of fluid and gas physics using only C and SDL.


## Lessons Learned:

Through this project, I learned how to visually debug complex simulations by continuously observing system behavior in the SDL-rendered window. This taught me how to identify logic errors, track unexpected state changes, and verify physics rules in real time.

I also became proficient with the SDL2 library, using it to build a fully interactive graphical environment. This included window creation, surface rendering, event handling, and efficient pixel updates—skills essential for low-level graphics and real-time simulation work.

In designing the simulation, I learned how to create a structured, scalable environment using a custom Cell data type to represent materials, fill levels, and spatial coordinates. This helped me understand how to model physical systems as discrete computational units and how to organize state transitions cleanly from frame to frame.

Overall, this project strengthened my skills in C programming, SDL graphics, simulation design, debugging methodology, and real-time system logic.
