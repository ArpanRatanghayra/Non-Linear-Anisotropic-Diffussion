#include "pm_ocl.hpp"
#include <iostream>
#include <fstream>      
#include <sstream>      
#include <iomanip>      
#include <cmath>        
#include <stdexcept>    
#include <algorithm>	
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
    #include "cl.hpp"
#else
    #include <CL/cl.hpp>
#endif

void pm_parallel(img_data *idata, proc_data *pdata, cl_data *cdata)
{
    cl_int err = CL_SUCCESS;
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if(!platforms.size()) {
        throw std::runtime_error("No OpenCL platforms were found!");
    }

    cl::Platform platform;

    if(cdata->platformId >= 0 && cdata->platformId < platforms.size()) {
        platform = platforms[cdata->platformId];
    } else {
        platform = platforms.front();
    }

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if(!devices.size()) {
        throw std::runtime_error("No OpenCL devices were found!");
    }

    cl::Device device;

    if(cdata->deviceId >= 0 && cdata->deviceId < devices.size()) {
        device = devices[cdata->deviceId];
    } else {
        std::vector<size_t> max_work_item_size = {0, 0, 0};
        std::vector<size_t> dev_work_item_size;
        for(auto d : devices) {
            d.getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &dev_work_item_size);
            if(dev_work_item_size[1] > max_work_item_size[1]) {
                max_work_item_size = dev_work_item_size;
                device = d;
            }
        }
    }

    std::vector<cl::Device> ds { device };
    cl::Context context(ds, NULL, NULL, NULL);
    cl::CommandQueue queue(context, device, (cdata->profile ? CL_QUEUE_PROFILING_ENABLE : 0));
    cl::Program program;

    if(cdata->bitcode) {
        auto binaries = cl::Program::Binaries {
            std::make_pair<const void *, ::size_t>(cdata->filename.c_str(),
                                                   cdata->filename.length())
        };
        program = cl::Program(context, ds, binaries);
    } else {
        std::ifstream in(cdata->filename);

        if(in.fail()) {
            throw std::invalid_argument(cdata->filename);
        }

        std::string source(
            (std::istreambuf_iterator<char> (in)),
            std::istreambuf_iterator<char> ());
        in.close();
        program = cl::Program(context, source, false);
    }

    cl_int status = program.build(ds);

    if(status == CL_BUILD_PROGRAM_FAILURE) {
        std::string build_log;
        program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &build_log);
        std::cerr << build_log << std::endl;
    }

    cl_ulong global_size;
    device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &global_size);

    if(global_size < idata->size * sizeof(uint)) {
        std::stringstream ss;
        std::string dname;
        device.getInfo(CL_DEVICE_NAME, &dname);
        ss << "Image size is too large, max available memory size for device "
           << dname << " is " << global_size << std::endl;
        throw std::runtime_error(ss.str());
    }

    cl::Buffer bits(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, idata->size * sizeof(uint), idata->bits);
    cl::Kernel kernel(program, "pm");
    auto pmKernel = cl::make_kernel<cl::Buffer &, float, int, float, int, int, int, int>(kernel);
    size_t max_work_group_size;
    device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &max_work_group_size);
    size_t work_group_x = std::min((size_t)idata->w, max_work_group_size);
    size_t work_group_y = std::min((size_t)idata->h, max_work_group_size);
    cl::NDRange workGroup(work_group_x, work_group_y);
	cl::EnqueueArgs enqueueArgs(queue, workGroup);

    if(cdata->verbose) {
        std::string pname, dname;
        platform.getInfo(CL_PLATFORM_NAME, &pname);
        device.getInfo(CL_DEVICE_NAME, &dname);
        std::cout << "selected platform: " << pname << std::endl;
        std::cout << "selected device: "   << dname << std::endl;
        std::cout << "work group size: " << work_group_x << ", " << work_group_y << std::endl;
        std::cout << "image size: " << idata->w << ", " << idata->h << std::endl;
    }

    double total_time = 0.0;
    int parts_x = ceil(idata->w / (float)max_work_group_size);
    int parts_y = ceil(idata->h / (float)max_work_group_size);
    int offset_x = 0, offset_y = 0;

    for(int py = 0; py < parts_y; ++py) {
        offset_y = py * work_group_y;

        for(int px = 0; px < parts_x; ++px) {
            offset_x = px * work_group_x;

			for (int it = 0; it < pdata->iterations; ++it) {
				queue.finish();

				if(cdata->profile) {
					cl::Event event = pmKernel(enqueueArgs, bits, pdata->thresh, pdata->conduction_func, pdata->lambda, idata->w, idata->h, offset_x, offset_y);
					event.wait();
					cl_ulong time_start, time_end;
					event.getProfilingInfo(CL_PROFILING_COMMAND_START, &time_start);
					event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);
					total_time += (time_end - time_start);
				} else {
					pmKernel(enqueueArgs, bits, pdata->thresh, pdata->conduction_func, pdata->lambda, idata->w, idata->h, offset_x, offset_y);
				}
			}
        }
    }

    if(cdata->profile) {
        std::cout << "parallel execution time in milliseconds = " << std::fixed
                  << std::setprecision(3) << (total_time / 1000000.0) << " ms" << std::endl;
    }
}