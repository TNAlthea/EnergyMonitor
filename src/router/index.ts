import { createRouter, createWebHistory } from 'vue-router'
import HomeView from '../views/HomeView.vue'

import DashboardPage from '@/views/pages/DashboardPage.vue'
import MonitorPage from '@/views/pages/MonitorPage.vue'
import SettingsPage from '@/views/pages/SettingsPage.vue'
import AnomalyLogPage from '@/views/pages/AnomalyLogPage.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'home',
      component: HomeView,
      redirect: '/Dashboard',
      children: [
        {
          path: 'Dashboard',
          component: DashboardPage
        },
        {
          path: 'Monitor',
          component: MonitorPage
        },
        {
          path: 'Settings',
          component: SettingsPage 
        },
        {
          path: 'AnomalyLog',
          component: AnomalyLogPage
        }
      ]
    },
  ]
})

export default router
