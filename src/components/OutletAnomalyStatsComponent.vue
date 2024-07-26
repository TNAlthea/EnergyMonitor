<script setup lang="ts">
import { ref, onMounted, defineProps, defineEmits, computed } from 'vue'
import {
  EyeSlashIcon,
  CalendarDaysIcon,
  ArrowUpCircleIcon,
  ScaleIcon,
  Bars3CenterLeftIcon
} from '@heroicons/vue/24/solid'

const emit = defineEmits(['current-id'])

const props = defineProps({
  anomalyData: {
    type: Array as () => {
      id: string
      data_id: string
      device_id: string
      if_labels_anomaly: string
      rf_labels_anomaly: string
      current: number
      voltage: number
      power: number
      energy: number
      frequency: number
      power_factor: number
      created_at: string
    }[],
    required: true
  }
})

const device_ids = ref<string[]>([])
const uniqueDeviceIds = ref([...new Set(props.anomalyData.map((item) => item.device_id))])

const deviceStats = ref<
  Map<
    string,
    {
      meanPower: number
      medianPower: number
      totalInstances: number
      latestTimestamp: string
      maxPower: number
    }
  >
>(new Map())

const calculateStats = async () => {
  const deviceDataMap: Record<string, { powers: number[]; timestamps: string[] }> = {}

  props.anomalyData.forEach(({ device_id, power, created_at }) => {
    if (!deviceDataMap[device_id]) {
      deviceDataMap[device_id] = { powers: [], timestamps: [] }
    }
    deviceDataMap[device_id].powers.push(power)
    deviceDataMap[device_id].timestamps.push(created_at)
  })

  Object.entries(deviceDataMap).forEach(([deviceId, { powers, timestamps }]) => {
    powers.sort((a, b) => a - b)
    const totalInstances = powers.length
    const meanPower = powers.reduce((acc, curr) => acc + curr, 0) / totalInstances
    const medianPower =
      totalInstances % 2 === 0
        ? (powers[totalInstances / 2 - 1] + powers[totalInstances / 2]) / 2
        : powers[Math.floor(totalInstances / 2)]
    const maxPower = Math.max(...powers)
    const latestTimestamp = timestamps.sort().pop() || ''

    deviceStats.value.set(deviceId, {
      meanPower,
      medianPower,
      maxPower,
      totalInstances,
      latestTimestamp
    })
  })
}

const formattedDate = (date: string) => {
  return new Date(date).toLocaleString('en-US', {
    year: 'numeric',
    month: 'long',
    day: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  })
}

const formattedValue = (value: number) => {
  return value.toFixed(2)
}

const emitAnomalyDataByDeviceId = (id: string) => {
  emit('current-id', id)
}

onMounted(async () => {
  await calculateStats()
})
</script>

<template>
  <div class="p-5 bg-cyan-50 bg-opacity-80 rounded-md">
    <div
      v-for="(item, index) in deviceStats"
      :key="index"
      class="p-3 bg-white rounded-md bg-opacity-60 flex flex-col gap-5"
    >
      <span
        class="flex flex-row items-end justify-end gap-5 p-3 border-b-2 border-gray-300 rounded-md"
      >
        <p class="text-gray-500">Anomaly Stats for Device ID:</p>
        <p class="text-2xl">{{ item[index] }}</p>
      </span>
      <div class="grid grid-cols-3 gap-5">
        <div class="p-3 border-2 border-gray-400 relative overflow-hidden">
          <p>Mean Power Value</p>
          <span class="font-semibold tracking-wider flex flex-row gap-2 items-end">
            <p class="text-4xl">
              {{ formattedValue(item[index + 1].meanPower) }}
            </p>
            <p class="text-xl">Watt</p>
          </span>
          <span class="absolute bottom-0 right-0 translate-y-3 translate-x-3 opacity-20">
            <ScaleIcon class="text-gray-500 h-16 w-16 md:h-20 md:w-20" />
          </span>
        </div>
        <div class="p-3 border-2 border-gray-400 relative overflow-hidden">
          <p>Median Power Value</p>
          <span class="font-semibold tracking-wider flex flex-row gap-2 items-end">
            <p class="text-4xl">
              {{ formattedValue(item[index + 1].medianPower) }}
            </p>
            <p class="text-xl">Watt</p>
          </span>
          <span class="absolute bottom-0 right-0 translate-y-3 translate-x-3 opacity-20">
            <Bars3CenterLeftIcon class="text-gray-500 h-16 w-16 md:h-20 md:w-20" />
          </span>
        </div>
        <div class="p-3 border-2 border-gray-400 relative overflow-hidden">
          <p>Max Power Value</p>
          <span class="font-semibold tracking-wider flex flex-row gap-2 items-end">
            <p class="text-4xl">
              {{ formattedValue(item[index + 1].maxPower) }}
            </p>
            <p class="text-lg">Watt</p>
          </span>
          <span class="absolute bottom-0 right-0 translate-y-3 translate-x-3 opacity-20">
            <ArrowUpCircleIcon class="text-gray-500 h-16 w-16 md:h-20 md:w-20" />
          </span>
        </div>
        <div class="p-3 border-2 border-gray-400 relative overflow-hidden col-span-2">
          <p>Number of Anomaly Instances</p>
          <span class="flex flex-row gap-2 items-end">
            <p class="text-4xl font-semibold tracking-wider">
              {{ item[index + 1].totalInstances }}
            </p>
            <p class="text-xl font-semibold tracking-wider">Instances</p>
            <p class="text-xs text-gray-500">(instances recorded every 5s)</p>
          </span>
          <span class="absolute bottom-0 right-0 translate-y-3 translate-x-3 opacity-20">
            <EyeSlashIcon class="text-gray-500 h-16 w-16 md:h-20 md:w-20" />
          </span>
        </div>
        <div class="p-3 border-2 border-gray-400 relative overflow-hidden">
          <p>Latest Instances</p>
          <span class="font-semibold tracking-wide">
            <p class="text-2xl">
              {{ formattedDate(item[index + 1].latestTimestamp) }}
            </p>
          </span>
          <span class="absolute bottom-0 right-0 translate-y-3 translate-x-3 opacity-20">
            <CalendarDaysIcon class="text-gray-500 h-16 w-16 md:h-20 md:w-20" />
          </span>
        </div>
      </div>
      <div class="flex justify-end items-end" @click="emitAnomalyDataByDeviceId(item[index])">
        <p class="text-blue-500 font-semibold">See Log Details</p>
      </div>
    </div>
  </div>
</template>
