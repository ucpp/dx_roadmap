#pragma once

#include <common/pch.h>

namespace engine
{
  using namespace Microsoft::WRL;

  class DeviceResources
  {
  public:
    static const uint8 num_frames = 3;

  private:
    void enableDebugLayer();
    bool checkTearingSupport();
    ComPtr<IDXGIAdapter4> getAdapter(bool use_warp);
    ComPtr<ID3D12Device2> createDevice(ComPtr<IDXGIAdapter4> adapter);
    ComPtr<ID3D12CommandQueue> createCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
    ComPtr<IDXGISwapChain4> createSwapChain(HWND hwnd, ComPtr<ID3D12CommandQueue> command_queue, uint32 width, uint32 height, uint32 buffer_count);
    ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 num_descriptors);
    ComPtr<ID3D12CommandAllocator> createCommandAllocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);

    void updateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swap_chain, ComPtr<ID3D12DescriptorHeap> descriptor_heap);

  private:
    bool is_initialized;

    ComPtr<ID3D12Device2> device;
    ComPtr<ID3D12CommandQueue> command_queue;
    ComPtr<IDXGISwapChain4> swap_chain;
    ComPtr<ID3D12Resource> back_buffers[num_frames];
    ComPtr<ID3D12GraphicsCommandList> command_list;
    ComPtr<ID3D12CommandAllocator> command_allocators[num_frames];
    ComPtr<ID3D12DescriptorHeap> RTV_descriptor_heap;

    uint RTV_descriptor_size;
    uint current_back_buffer_index;

    ComPtr<ID3D12Fence> fence;
    uint64 fence_value = 0;
    uint64 frame_fence_values[num_frames] = {};
    HANDLE fence_event;
  };
}