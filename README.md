# Real-Time-Strand-Based-Hair-Rendering-using-Guide-Hair-Interpolation-And-Depth-Peeling

This project is carried out for my Final Year Project (SW40106 PROJEK SAINTIFIK II). Project was carried out using C++ as programming language along with OpenGL library, and Visual Studio 2019 as the IDE.

Hair is an important element in CGI and digital animation. However, it has been proven to be challenging to simulate hair properties when rendering hair. Lighting model that accounts for more than one light scattering direction renders high quality hair strands in terms of visual appearance at the cost of performance, while overlapping hair strands produce incorrect occlusion when accounting for hair transparency. 

The aim of this project is to render hair geometry in real-time and adapt order-independent transparency on hair strands. The objectives of this project are:

i) to render hair geometry in real-time using guide hair interpolation.

ii) to adapt order-independent transparency for correct occlusion of hair strands using depth peeling. 


These objectives were evaluated through two tests. Performance tests were carried out using different experiments, which were different number of guide hair strands with a fixed total number of hair strands rendered, different hair type setups with a fixed total number of hair strands, and different total number of hair strands with a fixed number of strands per guide patch. The results from the performance tests carried out have shown that guide hairs have reduced the number of hair strands simulated. This has allowed the hair-rendering performance to achieve the benchmark requirements of between 30 fps or 33.33 ms and 60 fps or 16.67 ms. The proposed system was also able to render different types of hairstyles that consists of up to about 50000 strands in real-time. The second test involved visual comparison. Visual results show depth peeling rendered hairball geometry with proper occlusion, thus visual results obtained are better than that of traditional alpha blending. Both objectives of this project were achieved. 

In conclusion, this project has provided a technique to improve hair-rendering performance and adapt order-independent transparency when rendering explicit hair strands.
