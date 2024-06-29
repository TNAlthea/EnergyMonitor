<script setup lang="ts">
import mqtt from 'mqtt'
import { ref, computed, defineExpose } from 'vue'

interface ElectricalData {
  voltage: number
  current: number
  power: number
  energy: number
  frequency: number
  power_factor: number
  [key: string]: any
}

interface LastUpdateTimes {
  timestamp: Date
  [key: string]: any
}

interface ElectricalDataMap {
  [key: string]: ElectricalData
}

const props = defineProps({
  client: {
    type: mqtt.MqttClient,
    required: true
  },
  electricalData: {
    type: Object as () => ElectricalDataMap,
    required: true
  },
  lastUpdate: {
    type: Object as () => Record<string, LastUpdateTimes>,
    required: true
  }
})

const totalElectricalData = ref<ElectricalData>({
  voltage: 0,
  current: 0,
  power: 0,
  energy: 0,
  frequency: 0,
  power_factor: 0
})

const calculateTotalElectricalData = () => {
  resetTotalElectricalData()
  for (const key in props.electricalData) {
    const data = props.electricalData[key]
    totalElectricalData.value.voltage += data.voltage
    totalElectricalData.value.current += data.current
    totalElectricalData.value.power += data.power
    totalElectricalData.value.energy += data.energy
    totalElectricalData.value.frequency += data.frequency
    totalElectricalData.value.power_factor += data.power_factor
  }
}

const resetTotalElectricalData = () => {
  totalElectricalData.value = {
    voltage: 0,
    current: 0,
    power: 0,
    energy: 0,
    frequency: 0,
    power_factor: 0
  }
}

const adjustedTotalElectricalDataFormat = computed(() => {
  return {
    voltage: Number(totalElectricalData.value.voltage).toFixed(2),
    current: Number(totalElectricalData.value.current).toFixed(2),
    power: Number(totalElectricalData.value.power).toFixed(2),
    energy: Number(totalElectricalData.value.energy).toFixed(2),
    frequency: Number(totalElectricalData.value.frequency).toFixed(2),
    power_factor: Number(totalElectricalData.value.power_factor).toFixed(2)
  }
})

defineExpose({
  calculateTotalElectricalData,
  resetTotalElectricalData
})
</script>
<template>
  <div class="p-5 bg-cyan-50 bg-opacity-80 rounded-md flex flex-col gap-2">
    <p class="">Total Data Seluruh Perangkat</p>
    <section class="grid grid-cols-3 grid-flow-col gap-4" id="totalData">
      <div class="relative p-3 rounded-md bg-teal-50 overflow-hidden">
        <div class="flex flex-col">
          <p>Current:</p>
          <p v-if="props.electricalData" class="text-2xl font-semibold">
            {{ adjustedTotalElectricalDataFormat.current }}
          </p>
          <p v-else class="text-2xl font-semibold">No Data available</p>
        </div>
        <span class="absolute -bottom-2 -right-1">
          <p class="text-7xl font-semibold opacity-50">A</p>
        </span>
      </div>
      <div class="relative p-3 rounded-md bg-teal-50 overflow-hidden">
        <div class="flex flex-col">
          <p>Voltage:</p>
          <p v-if="props.electricalData" class="text-2xl font-semibold">
            {{ adjustedTotalElectricalDataFormat.voltage }}
          </p>
          <p v-else class="text-2xl font-semibold">No Data available</p>
        </div>
        <span class="absolute -bottom-2 -right-1">
          <p class="text-7xl font-semibold opacity-50">V</p>
        </span>
      </div>
      <div class="relative p-3 rounded-md bg-teal-50 overflow-hidden">
        <div class="flex flex-col">
          <p>Power:</p>
          <p v-if="props.electricalData" class="text-2xl font-semibold">
            {{ adjustedTotalElectricalDataFormat.power }}
          </p>
          <p v-else class="text-2xl font-semibold">No Data available</p>
        </div>
        <span class="absolute -bottom-2 -right-1">
          <p class="text-7xl font-semibold opacity-50">W</p>
        </span>
      </div>
    </section>
  </div>
</template>
