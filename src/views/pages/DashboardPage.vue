<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import mqtt from 'mqtt'
import ElectricalDataComponent from '@/components/ElectricalDataComponent.vue'
import OutletControlComponent from '@/components/OutletControlComponent.vue'
import TotalUsageComponent from '@/components/TotalUsageComponent.vue'

const props = defineProps({
  client: {
    type: mqtt.MqttClient,
    required: true
  }
})
</script>

<template>
  <div class="flex flex-col gap-5">
    <ElectricalDataComponent :client="client" />
    <TotalUsageComponent :client="client" />
    <OutletControlComponent :client="client" />
  </div>
</template>
