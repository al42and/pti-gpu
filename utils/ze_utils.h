//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#ifndef PTI_UTILS_ZE_UTILS_H_
#define PTI_UTILS_ZE_UTILS_H_

#include <string.h>

#include <string>
#include <vector>

#include <level_zero/ze_api.h>
#include <level_zero/zet_api.h>

#include "pti_assert.h"
#include "utils.h"

namespace utils {
namespace ze {

inline std::vector<ze_driver_handle_t> GetDriverList() {
  ze_result_t status = ZE_RESULT_SUCCESS;

  uint32_t driver_count = 0;
  status = zeDriverGet(&driver_count, nullptr);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  if (driver_count == 0) {
    return std::vector<ze_driver_handle_t>();
  }

  std::vector<ze_driver_handle_t> driver_list(driver_count);
  status = zeDriverGet(&driver_count, driver_list.data());
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  return driver_list;
}

inline std::vector<ze_device_handle_t> GetDeviceList(ze_driver_handle_t driver) {
  PTI_ASSERT(driver != nullptr);
  ze_result_t status = ZE_RESULT_SUCCESS;

  uint32_t device_count = 0;
  status = zeDeviceGet(driver, &device_count, nullptr);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  if (device_count == 0) {
    return std::vector<ze_device_handle_t>();
  }

  std::vector<ze_device_handle_t> device_list(device_count);
  status = zeDeviceGet(driver, &device_count, device_list.data());
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  return device_list;
}

inline ze_driver_handle_t GetGpuDriver() {
  for (auto driver : GetDriverList()) {
    for (auto device : GetDeviceList(driver)) {
      ze_device_properties_t props{ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES, };
      ze_result_t status = zeDeviceGetProperties(device, &props);
      PTI_ASSERT(status == ZE_RESULT_SUCCESS);
      if (props.type == ZE_DEVICE_TYPE_GPU) {
        return driver;
      }
    }
  }

  return nullptr;
}

inline ze_device_handle_t GetGpuDevice() {
  for (auto driver : GetDriverList()) {
    for (auto device : GetDeviceList(driver)) {
      ze_device_properties_t props{ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES, };
      ze_result_t status = zeDeviceGetProperties(device, &props);
      PTI_ASSERT(status == ZE_RESULT_SUCCESS);
      if (props.type == ZE_DEVICE_TYPE_GPU) {
        return device;
      }
    }
  }

  return nullptr;
}

inline ze_context_handle_t GetContext(ze_driver_handle_t driver) {
  PTI_ASSERT(driver != nullptr);

  ze_result_t status = ZE_RESULT_SUCCESS;
  ze_context_handle_t context = nullptr;
  ze_context_desc_t context_desc = {
      ZE_STRUCTURE_TYPE_CONTEXT_DESC, nullptr, 0};

  status = zeContextCreate(driver, &context_desc, &context);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  return context;
}

inline std::string GetDeviceName(ze_device_handle_t device) {
  PTI_ASSERT(device != nullptr);
  ze_result_t status = ZE_RESULT_SUCCESS;
  ze_device_properties_t props{};
  props.stype = ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
  status = zeDeviceGetProperties(device, &props);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  return props.name;
}

inline int GetMetricId(zet_metric_group_handle_t group, std::string name) {
  PTI_ASSERT(group != nullptr);

  ze_result_t status = ZE_RESULT_SUCCESS;
  uint32_t metric_count = 0;
  status = zetMetricGet(group, &metric_count, nullptr);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  if (metric_count == 0) {
    return -1;
  }

  std::vector<zet_metric_handle_t> metric_list(metric_count, nullptr);
  status = zetMetricGet(group, &metric_count, metric_list.data());
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  int target = -1;
  for (uint32_t i = 0; i < metric_count; ++i) {
    zet_metric_properties_t metric_props{};
    status = zetMetricGetProperties(metric_list[i], &metric_props);
    PTI_ASSERT(status == ZE_RESULT_SUCCESS);

    if (name == metric_props.name) {
      target = i;
      break;
    }
  }

  return target;
}

inline zet_metric_group_handle_t FindMetricGroup(
    ze_device_handle_t device, std::string name,
    zet_metric_group_sampling_type_flag_t type) {
  PTI_ASSERT(device != nullptr);
  
  ze_result_t status = ZE_RESULT_SUCCESS;
  uint32_t group_count = 0;
  status = zetMetricGroupGet(device, &group_count, nullptr);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  if (group_count == 0) {
    return nullptr;
  }

  std::vector<zet_metric_group_handle_t> group_list(group_count, nullptr);
  status = zetMetricGroupGet(device, &group_count, group_list.data());
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  zet_metric_group_handle_t target = nullptr;
  for (uint32_t i = 0; i < group_count; ++i) {
    zet_metric_group_properties_t group_props{};
    group_props.stype = ZET_STRUCTURE_TYPE_METRIC_GROUP_PROPERTIES;
    status = zetMetricGroupGetProperties(group_list[i], &group_props);
    PTI_ASSERT(status == ZE_RESULT_SUCCESS);

    if (name == group_props.name && (group_props.samplingType & type)) {
      target = group_list[i];
      break;
    }
  }

  return target;
}

inline size_t GetKernelMaxSubgroupSize(ze_kernel_handle_t kernel) {
  PTI_ASSERT(kernel != nullptr);
  ze_kernel_properties_t props{};
  ze_result_t status = zeKernelGetProperties(kernel, &props);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  return props.maxSubgroupSize;
}

inline std::string GetKernelName(ze_kernel_handle_t kernel) {
  PTI_ASSERT(kernel != nullptr);

  size_t size = 0;
  ze_result_t status = zeKernelGetName(kernel, &size, nullptr);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  PTI_ASSERT(size > 0);

  std::vector<char> name(size);
  status = zeKernelGetName(kernel, &size, name.data());
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  PTI_ASSERT(name[size - 1] == '\0');
  return std::string(name.begin(), name.end() - 1);
}

inline uint64_t GetDeviceTimestamp(ze_device_handle_t device) {
  PTI_ASSERT(device != nullptr);
  uint64_t host_timestamp = 0, device_timestamp = 0;
  ze_result_t status = zeDeviceGetGlobalTimestamps(
      device, &host_timestamp, &device_timestamp);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  return device_timestamp;
}

inline uint64_t GetDeviceTimerFrequency(ze_device_handle_t device) {
  PTI_ASSERT(device != nullptr);
  ze_device_properties_t props{ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES, };
  ze_result_t status = zeDeviceGetProperties(device, &props);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  return props.timerResolution;
}

inline ze_api_version_t GetDriverVersion(ze_driver_handle_t driver) {
  PTI_ASSERT(driver != nullptr);

  ze_api_version_t version = ZE_API_VERSION_FORCE_UINT32;
  ze_result_t status = zeDriverGetApiVersion(driver, &version);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  return version;
}

inline ze_api_version_t GetVersion() {
  auto driver_list = GetDriverList();
  if (driver_list.empty()) {
    return ZE_API_VERSION_FORCE_UINT32;
  }
  return GetDriverVersion(driver_list.front());
}

inline std::vector<ze_device_handle_t> GetSubDeviceList(
    ze_device_handle_t device) {
  PTI_ASSERT(device != nullptr);
  ze_result_t status = ZE_RESULT_SUCCESS;

  uint32_t sub_device_count = 0;
  status = zeDeviceGetSubDevices(device, &sub_device_count, nullptr);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  if (sub_device_count == 0) {
    return std::vector<ze_device_handle_t>();
  }

  std::vector<ze_device_handle_t> sub_device_list(sub_device_count);
  status = zeDeviceGetSubDevices(
      device, &sub_device_count, sub_device_list.data());
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  return sub_device_list;
}

} // namespace ze
} // namespace utils

#endif // PTI_UTILS_ZE_UTILS_H_