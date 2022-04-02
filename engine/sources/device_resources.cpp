#include <device_resources.h>

namespace engine
{
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

}