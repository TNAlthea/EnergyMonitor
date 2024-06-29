<script setup>
import { onMounted, ref, computed } from 'vue'
import axios from 'axios'
import { Bar } from 'vue-chartjs'
import {
  Chart as ChartJS,
  Title,
  Tooltip,
  Legend,
  BarElement,
  CategoryScale,
  LinearScale
} from 'chart.js'

ChartJS.register(Title, Tooltip, Legend, BarElement, CategoryScale, LinearScale)

onMounted(() => {
  getDailyPowerData(), window.addEventListener('resize', updateWindowWidth)
})

const windowWidth = ref(window.innerWidth)

const updateWindowWidth = () => {
  windowWidth.value = window.innerWidth
}

const isMobile = computed(() => windowWidth.value < 1024)
const style = computed(() => ({
  width: isMobile.value ? '400px' : '1200px',
  height: isMobile.value ? '300px' : '500px'
}))

const year = new Date().getFullYear() // Assuming you want the current year
const month = new Date().getMonth() + 1
const daysInMonth = new Date(year, month, 0).getDate()
const allDates = Array.from({ length: daysInMonth }, (_, i) =>
  new Date(year, month - 1, i + 1).toLocaleDateString()
)

const chartData = ref({
  labels: allDates,
  datasets: [{ data: [], label: 'Total Daya/Power (W)', backgroundColor: '#3182CE' }]
})

const chartOptions = {
  responsive: true
}

const loaded = ref(false)
const getDailyPowerData = async () => {
  try {
    const baseUrl = import.meta.env.VITE_GET_TOTAL_POWER_DATA_DAILY
    const url = baseUrl.replace(':month', month)
    const response = await axios.get(url)

    const responseData = response.data.data

    // Create a map for quick lookup
    const dataMap = new Map(
      responseData.map((item) => [new Date(item.date).toLocaleDateString(), item.total_power])
    )

    // Populate dataset data, using data from the map or defaulting to 0
    chartData.value.datasets[0].data = allDates.map((date) => dataMap.get(date) || 0)
    loaded.value = true
  } catch (error) {
    console.error(error)
  }
}
</script>

<template>
  <div class="bg-white bg-opacity-80 rounded-lg flex flex-col justify-center">
    <div class="px-5 pt-5">
      <p>Tren Pemakaian Listrik Bulan Ini</p>
    </div>
    <div class="bg-white rounded-lg p-5 m-5 flex justify-center">
      <Bar v-if="loaded" :data="chartData" :style="style" />
      <p v-else>Fetching data...</p>
    </div>
  </div>
</template>
