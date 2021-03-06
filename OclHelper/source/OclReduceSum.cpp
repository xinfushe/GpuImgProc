#include "OclReduceSum.h"
#include "OclUtils.h"

#include <sstream>

#define OCL_PROGRAM_SOURCE(s) #s

using namespace Ocl;

const char ReduceSum::sSource[] = OCL_PROGRAM_SOURCE(
inline void block_reduce_sum(local volatile int sh_data[SH_MEM_SIZE])
{
    if (get_local_id(0) < 128) sh_data[get_local_id(0)] += sh_data[128+get_local_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);

    if (get_local_id(0) < 64) sh_data[get_local_id(0)] += sh_data[64+get_local_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);

    if (get_local_id(0) < 32) sh_data[get_local_id(0)] += sh_data[32 + get_local_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);

    if (get_local_id(0) < 16) sh_data[get_local_id(0)] += sh_data[16 + get_local_id(0)];
    if (get_local_id(0) < 8) sh_data[get_local_id(0)] += sh_data[8 + get_local_id(0)];
    if (get_local_id(0) < 4) sh_data[get_local_id(0)] += sh_data[4 + get_local_id(0)];
    if (get_local_id(0) < 2) sh_data[get_local_id(0)] += sh_data[2 + get_local_id(0)];
    if (get_local_id(0) < 1) sh_data[get_local_id(0)] += sh_data[1 + get_local_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);
}

kernel void reduce_sum(global const int* p_data, int count, global int* p_block_sum)
{
    local int sh_data[SH_MEM_SIZE];
    size_t index = get_global_id(0);
    sh_data[get_local_id(0)] = (index < count)?p_data[index]:0;
    index += get_global_size(0);
    sh_data[get_local_id(0)] += (index < count)?p_data[index]:0;
    index += get_global_size(0);
    sh_data[get_local_id(0)] += (index < count)?p_data[index]:0;
    index += get_global_size(0);
    sh_data[get_local_id(0)] += (index < count)?p_data[index]:0;
    barrier(CLK_LOCAL_MEM_FENCE);
    block_reduce_sum(sh_data);
    if (get_local_id(0) == 0)
    {
        p_block_sum[get_group_id(0)] = sh_data[0];
    }
}
);

ReduceSum::ReduceSum(const cl::Context& ctxt)
    :mDepth(8),
     mBlkSize(1<<8),
     mContext(ctxt)
{
    std::ostringstream options;
    options << " -DSH_MEM_SIZE=" << (1<<8);

    cl::Program::Sources source(1, std::make_pair(sSource, strlen(sSource)));
    mProgram = cl::Program(mContext, source);
    mProgram.build(options.str().c_str());

    mKernel = cl::Kernel(mProgram, "reduce_sum");
}

ReduceSum::~ReduceSum()
{
}

void ReduceSum::createIntBuffer(size_t buffSize)
{
    size_t memSize = 0;
    size_t intBuffSize = (buffSize/mBlkSize)+(((buffSize%mBlkSize)==0)?0:1);
    if (mBuff.get() != 0)
    {
        memSize = mBuff->count();
    }

    if (memSize < intBuffSize)
    {
        mBuff.reset(new Ocl::DataBuffer<int>(mContext, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, intBuffSize));
    }
}

int ReduceSum::process(const cl::CommandQueue& queue, Ocl::DataBuffer<int>& buffer)
{
    cl::Event event;
    size_t buffSize = buffer.count();

    createIntBuffer(buffSize);

    mKernel.setArg(0, buffer.buffer());
    mKernel.setArg(1, (int)buffSize);
    mKernel.setArg(2, mBuff->buffer());

    size_t buffSize1 = buffSize/4;
    size_t globalSize = ((buffSize1/mBlkSize)+(((buffSize1%mBlkSize) == 0)?0:1))*mBlkSize;
    queue.enqueueNDRangeKernel(mKernel, cl::NullRange, cl::NDRange(globalSize), cl::NDRange(mBlkSize), NULL, &event);
    event.wait();
    size_t time = kernelExecTime(queue, event);

    size_t groupCount = (buffSize/mBlkSize)+(((buffSize%mBlkSize)==0)?0:1);
    while (groupCount > 1)
    {
        mKernel.setArg(0, mBuff->buffer());
        mKernel.setArg(1, (int)groupCount);
        mKernel.setArg(2, mBuff->buffer());
        size_t groupCount1 = (groupCount > 4)?(groupCount/4):1;
        globalSize = ((groupCount1/mBlkSize)+(((groupCount1%mBlkSize)==0)?0:1))*mBlkSize;
        queue.enqueueNDRangeKernel(mKernel, cl::NullRange, cl::NDRange(globalSize), cl::NDRange(mBlkSize), NULL, &event);
        event.wait();
        time += kernelExecTime(queue, event);
        groupCount = globalSize/mBlkSize;
    }

    int retValue;
    int* pData = mBuff->map(queue, CL_TRUE, CL_MAP_READ, 0, 1);
    retValue = *pData;
    mBuff->unmap(queue, pData);

    return retValue;
}
