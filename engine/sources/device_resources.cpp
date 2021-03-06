#include <device_resources.h>
#include <window.h>

namespace engine
{
  void DeviceResources::loadPipeline(const Window& window)
  {
    ComPtr<IDXGIAdapter4> dxgi_adapter4 = getAdapter(window.getUseWarp());
    device = createDevice(dxgi_adapter4);
    command_queue = createCommandQueue(device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    swap_chain = createSwapChain(window.getHwnd(), command_queue, window.getWidth(), window.getHeight(), num_frames);
    current_back_buffer_index = swap_chain->GetCurrentBackBufferIndex();

    RTV_descriptor_heap = createDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, num_frames);
    RTV_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    updateRenderTargetViews(device, swap_chain, RTV_descriptor_heap);

    for (int frame = 0; frame < num_frames; ++frame)
    {
      command_allocators[frame] = createCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    }
    command_list = createCommandList(device, command_allocators[current_back_buffer_index], D3D12_COMMAND_LIST_TYPE_DIRECT);

    fence = createFence(device);
    fence_event = createEventHandle();

    is_initialized = true;
  }

  void DeviceResources::render(bool vsync, bool tearing_supported)
  {
    auto command_allocator = command_allocators[current_back_buffer_index];
    auto back_buffer = back_buffers[current_back_buffer_index];
    command_allocator->Reset();
    command_list->Reset(command_allocator.Get(), nullptr);

    // Clear the render target.
    {
      CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(back_buffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
      command_list->ResourceBarrier(1, &barrier);

      FLOAT clear_color[] = { 0.2f, 0.2f, 0.2f, 1.0f };
      CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(RTV_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), current_back_buffer_index, RTV_descriptor_size);
      command_list->ClearRenderTargetView(rtv, clear_color, 0, nullptr);
    }

    // Present
    {
      CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(back_buffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
      command_list->ResourceBarrier(1, &barrier);
      ThrowIfFailed(command_list->Close());

      ID3D12CommandList* const command_lists[] = { command_list.Get() };
      command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);
      frame_fence_values[current_back_buffer_index] = signal(command_queue, fence, fence_value);

      UINT sync_interval = vsync ? 1 : 0;
      UINT present_flags = tearing_supported && !vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;
      ThrowIfFailed(swap_chain->Present(sync_interval, present_flags));

      current_back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
      waitForFenceValue(fence, frame_fence_values[current_back_buffer_index], fence_event);
    }
  }

  void DeviceResources::resize(uint32 width, uint32 height)
  {
    flush();

    for (int frame = 0; frame < DeviceResources::num_frames; ++frame)
    {
      back_buffers[frame].Reset();
      frame_fence_values[frame] = frame_fence_values[current_back_buffer_index];
    }

    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    ThrowIfFailed(swap_chain->GetDesc(&swap_chain_desc));
    ThrowIfFailed(swap_chain->ResizeBuffers(num_frames, width, height, swap_chain_desc.BufferDesc.Format, swap_chain_desc.Flags));

    current_back_buffer_index = swap_chain->GetCurrentBackBufferIndex();
    updateRenderTargetViews(device, swap_chain, RTV_descriptor_heap);
  }

  void DeviceResources::flush()
  {
    flush(command_queue, fence, fence_value, fence_event);
  }

  void DeviceResources::enableDebugLayer()
  {
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debug_interface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)));
    debug_interface->EnableDebugLayer();
#endif
  }

  bool DeviceResources::checkTearingSupport()
  {
    bool allow_tearing = false;

    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
      ComPtr<IDXGIFactory5> factory5;
      if (SUCCEEDED(factory4.As(&factory5)))
      {
        if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing))))
        {
          allow_tearing = false;
        }
      }
    }

    return allow_tearing;
  }

  ComPtr<IDXGIAdapter4> DeviceResources::getAdapter(bool use_warp)
  {
    ComPtr<IDXGIFactory4> dxgi_factory;
    uint create_factory_flags = 0;
#if defined(_DEBUG)
    create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    ThrowIfFailed(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&dxgi_factory)));

    ComPtr<IDXGIAdapter1> dxgi_adapter1;
    ComPtr<IDXGIAdapter4> dxgi_adapter4;

    if (use_warp)
    {
      ThrowIfFailed(dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgi_adapter1)));
      ThrowIfFailed(dxgi_adapter1.As(&dxgi_adapter4));
    }
    else
    {
      SIZE_T max_dedicated_video_memory = 0;
      for (uint i = 0; dxgi_factory->EnumAdapters1(i, &dxgi_adapter1) != DXGI_ERROR_NOT_FOUND; ++i)
      {
        DXGI_ADAPTER_DESC1 dxgi_adapter_desc1;
        dxgi_adapter1->GetDesc1(&dxgi_adapter_desc1);

        if ((dxgi_adapter_desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 
          && SUCCEEDED(D3D12CreateDevice(dxgi_adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) 
          && dxgi_adapter_desc1.DedicatedVideoMemory > max_dedicated_video_memory)
        {
          max_dedicated_video_memory = dxgi_adapter_desc1.DedicatedVideoMemory;
          ThrowIfFailed(dxgi_adapter1.As(&dxgi_adapter4));
        }
      }
    }

    return dxgi_adapter4;
  }

  ComPtr<ID3D12Device2> DeviceResources::createDevice(ComPtr<IDXGIAdapter4> adapter)
  {
    ComPtr<ID3D12Device2> d3d12_device2;
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12_device2)));
    
    // enable debug messages in debug mode
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> info_queue;
    if (SUCCEEDED(d3d12_device2.As(&info_queue)))
    {
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

      // Suppress whole categories of messages
      //D3D12_MESSAGE_CATEGORY categories[] = {};

      // Suppress messages based on their severity level
      D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

      // Suppress individual messages by their ID
      D3D12_MESSAGE_ID deny_ids[] = {
          D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
          D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
          D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
      };

      D3D12_INFO_QUEUE_FILTER info_filter = {};
      //info_filter.DenyList.NumCategories = _countof(categories);
      //info_filter.DenyList.pCategoryList = categories;
      info_filter.DenyList.NumSeverities = _countof(severities);
      info_filter.DenyList.pSeverityList = severities;
      info_filter.DenyList.NumIDs = _countof(deny_ids);
      info_filter.DenyList.pIDList = deny_ids;

      ThrowIfFailed(info_queue->PushStorageFilter(&info_filter));
    }
#endif

    return d3d12_device2;
  }

  ComPtr<ID3D12CommandQueue> DeviceResources::createCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
  {
    ComPtr<ID3D12CommandQueue> d3d12_command_queue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12_command_queue)));

    return d3d12_command_queue;
  }

  ComPtr<IDXGISwapChain4> DeviceResources::createSwapChain(HWND hwnd, ComPtr<ID3D12CommandQueue> command_queue, uint32_t width, uint32_t height, uint32_t buffer_count)
  {
    ComPtr<IDXGISwapChain4> dxgi_swap_chain4;
    ComPtr<IDXGIFactory4> dxgi_factory4;
    UINT create_factory_flags = 0;
#if defined(_DEBUG)
    create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&dxgi_factory4)));

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
    swap_chain_desc.Width = width;
    swap_chain_desc.Height = height;
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.Stereo = FALSE;
    swap_chain_desc.SampleDesc = { 1, 0 };
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = buffer_count;
    swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swap_chain_desc.Flags = checkTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swap_chain1;
    ThrowIfFailed(dxgi_factory4->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &swap_chain_desc, nullptr, nullptr, &swap_chain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    ThrowIfFailed(dxgi_factory4->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swap_chain1.As(&dxgi_swap_chain4));

    return dxgi_swap_chain4;
  }

  ComPtr<ID3D12DescriptorHeap> DeviceResources::createDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 num_descriptors)
  {
    ComPtr<ID3D12DescriptorHeap> descriptor_heap;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = num_descriptors;
    desc.Type = type;

    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptor_heap)));

    return descriptor_heap;
  }

  void DeviceResources::updateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swap_chain, ComPtr<ID3D12DescriptorHeap> descriptor_heap)
  {
    auto rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < num_frames; ++i)
    {
      ComPtr<ID3D12Resource> back_buffer;
      ThrowIfFailed(swap_chain->GetBuffer(i, IID_PPV_ARGS(&back_buffer)));

      device->CreateRenderTargetView(back_buffer.Get(), nullptr, rtv_handle);

      back_buffers[i] = back_buffer;

      rtv_handle.Offset(rtv_descriptor_size);
    }
  }

  ComPtr<ID3D12CommandAllocator> DeviceResources::createCommandAllocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
  {
    ComPtr<ID3D12CommandAllocator> command_allocator;
    ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator)));

    return command_allocator;
  }

  ComPtr<ID3D12GraphicsCommandList> DeviceResources::createCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> command_allocator, D3D12_COMMAND_LIST_TYPE type)
  {
    ComPtr<ID3D12GraphicsCommandList> command_list;
    ThrowIfFailed(device->CreateCommandList(0, type, command_allocator.Get(), nullptr, IID_PPV_ARGS(&command_list)));

    ThrowIfFailed(command_list->Close());

    return command_list;
  }

  ComPtr<ID3D12Fence> DeviceResources::createFence(ComPtr<ID3D12Device2> device)
  {
    ComPtr<ID3D12Fence> fence;

    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
  }

  HANDLE DeviceResources::createEventHandle()
  {
    HANDLE fence_event;

    fence_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fence_event && "Failed to create fence event.");

    return fence_event;
  }

  uint64 DeviceResources::signal(ComPtr<ID3D12CommandQueue> command_queue, ComPtr<ID3D12Fence> fence, uint64& fence_value)
  {
    uint64 fence_value_for_signal = ++fence_value;
    ThrowIfFailed(command_queue->Signal(fence.Get(), fence_value_for_signal));

    return fence_value_for_signal;
  }

  void DeviceResources::waitForFenceValue(ComPtr<ID3D12Fence> fence, uint64 fence_value, HANDLE fence_event, std::chrono::milliseconds duration)
  {
    if (fence->GetCompletedValue() < fence_value)
    {
      ThrowIfFailed(fence->SetEventOnCompletion(fence_value, fence_event));
      ::WaitForSingleObject(fence_event, static_cast<DWORD>(duration.count()));
    }
  }

  void DeviceResources::flush(ComPtr<ID3D12CommandQueue> command_queue, ComPtr<ID3D12Fence> fence, uint64& fence_value, HANDLE fence_event)
  {
    uint64 fenceValueForSignal = signal(command_queue, fence, fence_value);
    waitForFenceValue(fence, fenceValueForSignal, fence_event);
  }
}