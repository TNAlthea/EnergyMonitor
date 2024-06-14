<script setup lang="ts">
import { ref, onMounted } from 'vue'
onMounted(() => {
    console.log('TestComponent mounted')
})
</script>

<template>
  <main>
    <h1>Test Component</h1>
  </main>
</template>