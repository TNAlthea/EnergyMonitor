<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import mqtt from 'mqtt'
import OutletControlComponent from '@/components/OutletControlComponent.vue';

interface ElectricalData {
  voltage: number
  current: number
  power: number
  energy: number
  frequency: number
  power_factor: number
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
})
</script>

<template>
  <OutletControlComponent :client="client" :electricalData="props.electricalData" />
</template>
