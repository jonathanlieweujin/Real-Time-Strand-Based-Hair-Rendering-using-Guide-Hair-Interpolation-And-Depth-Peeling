# Real Time Strand-Based Hair Rendering using Guide Hair Interpolation And Depth-Peeling

Hair is an important element in CGI and digital animation. However, it has been proven to be challenging to simulate hair properties when rendering hair. Lighting model that accounts for more than one light scattering direction renders high quality hair strands in terms of visual appearance at the cost of performance, while overlapping hair strands produce incorrect occlusion when accounting for hair transparency. 

The aim of this project is to render hair geometry in real-time and adapt order-independent transparency on hair strands. The objectives of this project are:

i) to render hair geometry in real-time using guide hair interpolation.

ii) to adapt order-independent transparency for correct occlusion of hair strands using depth peeling. 


These objectives were evaluated through two tests. 

# Hair Rendering Performance
Performance tests were carried out using different experiments, which were different number of guide hair strands with a fixed total number of hair strands rendered, different hair type setups with a fixed total number of hair strands, and different total number of hair strands with a fixed number of strands per guide patch. The results from the performance tests carried out have shown that guide hairs have reduced the number of hair strands simulated. This has allowed the hair-rendering performance to achieve the benchmark requirements of between 30 fps or 33.33 ms and 60 fps or 16.67 ms. The proposed system was also able to render different types of hairstyles that consists of up to about 50000 strands in real-time. 

![image](https://github.com/jonathanlieweujin/Real-Time-Strand-Based-Hair-Rendering-using-Guide-Hair-Interpolation-And-Depth-Peeling/assets/106479441/f0e1dc06-4f0c-43ef-aa1a-4ae61be934c1)

Rendering performance with increasing number of guide hairs used. Total number of hair strands are kept constant at 25600 strands.

![image](https://github.com/jonathanlieweujin/Real-Time-Strand-Based-Hair-Rendering-using-Guide-Hair-Interpolation-And-Depth-Peeling/assets/106479441/c17d7f9a-29cb-40be-b898-2b2323f02275)

Average frame times of different hair type setups. Total number of hair strands are kept constant at 25600 strands.

![image](https://github.com/jonathanlieweujin/Real-Time-Strand-Based-Hair-Rendering-using-Guide-Hair-Interpolation-And-Depth-Peeling/assets/106479441/f3ea3950-dc23-4591-b7c4-d1b07145e8dc)

Rendering performance with increasing total number of hair strands used. Total number of strands per guide patch are kept constant at 64 strands.


# Visual Results of Depth Peeling
The second test involved visual comparison. Visual results show depth peeling rendered hairball geometry with proper occlusion, thus visual results obtained are better than that of traditional alpha blending. Both objectives of this project were achieved. 

![image](https://github.com/jonathanlieweujin/Real-Time-Strand-Based-Hair-Rendering-using-Guide-Hair-Interpolation-And-Depth-Peeling/assets/106479441/7c4036fe-2542-4460-8e05-0012de6368e1)

Close-up view of hair geometry rendered without (left) and with (right) depth peeling.

![image](https://github.com/jonathanlieweujin/Real-Time-Strand-Based-Hair-Rendering-using-Guide-Hair-Interpolation-And-Depth-Peeling/assets/106479441/63716a60-8358-4a5e-acfe-0a3ecc7935a5)

Full hairball geometry rendered without (left) and with (right) depth peeling.


![image](https://github.com/jonathanlieweujin/Real-Time-Strand-Based-Hair-Rendering-using-Guide-Hair-Interpolation-And-Depth-Peeling/assets/106479441/72ca8211-1bd0-41b2-a5ac-858794f914c2)

Zoomed in frame of rendered hairball geometry with alpha blending transparency. Hair geometry is rendered with incorrect occlusion as hair fragments are not sorted per pixel.

![image](https://github.com/jonathanlieweujin/Real-Time-Strand-Based-Hair-Rendering-using-Guide-Hair-Interpolation-And-Depth-Peeling/assets/106479441/fa72abff-9e91-4a5e-993c-c51694683cd1)

Zoomed in frame of rendered hairball geometry with depth-peeling transparency. Hair geometry is rendered after sorting hair fragments per pixel.


In conclusion, this project has provided a technique to improve hair-rendering performance and adapt order-independent transparency when rendering explicit hair strands.


Thorough details on the research framework, methodology and results can be found in the uploaded dissertation.
