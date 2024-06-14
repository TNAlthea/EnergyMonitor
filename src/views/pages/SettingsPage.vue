<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import mqtt from 'mqtt'
const props = defineProps({
  client: {
    type: mqtt.MqttClient,
    required: true
  }
})
</script>

<template>
    C
</template>
