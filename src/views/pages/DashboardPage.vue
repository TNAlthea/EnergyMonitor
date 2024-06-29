<script setup lang="ts">
import { ref, defineExpose, onMounted, onBeforeUnmount, watchEffect, onUnmounted } from 'vue'
import mqtt from 'mqtt'
import { eventBus } from '../../utils/eventBus.js'
import ElectricalDataComponent from '@/components/ElectricalDataComponent.vue'
import TotalUsageComponent from '@/components/TotalUsageComponent.vue'
import PowerUsageChartComponent from '@/components/PowerUsageChartComponent.vue'

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

const ElectricalDataRef = ref(ElectricalDataComponent ? ElectricalDataComponent : null)
let stopWatcher: Function | null = null
onMounted(() => {
  stopWatcher = watchEffect(() => {
    if (eventBus.triggerCalculateTotalElectricalData) {
      callCalculateTotalElectricalData()
      eventBus.triggerCalculateTotalElectricalData = false
    }
  })
})

onBeforeUnmount(() => {
  if (stopWatcher) {
    stopWatcher()
  }
})

onUnmounted(() => {
  watchEffect(() => {
    if (ElectricalDataRef.value) {
      ElectricalDataRef.value.resetTotalElectricalData()
    }
  })
  // Reset the trigger
  eventBus.triggerCalculateTotalElectricalData = false
})

const callCalculateTotalElectricalData = () => {
  if (ElectricalDataRef.value) {
    ElectricalDataRef.value.calculateTotalElectricalData()
  }
}

const callTotalResetElectricalData = () => {
  if (ElectricalDataRef.value) {
    ElectricalDataRef.value.resetTotalElectricalData()
  }
}

defineExpose({
  callCalculateTotalElectricalData,
  callTotalResetElectricalData
})
</script>

<template>
  <div class="flex flex-col gap-5">
    <ElectricalDataComponent
      :client="client"
      :electricalData="props.electricalData"
      :lastUpdate="props.lastUpdate"
      ref="ElectricalDataRef"
    />
    <TotalUsageComponent :client="client" />
    <PowerUsageChartComponent />
  </div>
</template>
