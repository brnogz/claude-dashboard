package com.claude.dashboard

import android.app.Application
import android.content.Context
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.claude.dashboard.data.DashboardRepository
import com.claude.dashboard.data.StatsResponse
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import androidx.core.content.edit

data class DashboardUiState(
    val stats: StatsResponse? = null,
    val error: String? = null,
    val isConnected: Boolean = false,
    val host: String = "192.168.1.110",
    val port: Int = 8080,
)

class DashboardViewModel(app: Application) : AndroidViewModel(app) {

    private val repo = DashboardRepository()
    private val prefs = app.getSharedPreferences("dashboard", Context.MODE_PRIVATE)

    private val _state = MutableStateFlow(DashboardUiState())
    val state: StateFlow<DashboardUiState> = _state

    init {
        _state.value = _state.value.copy(
            host = prefs.getString("host", "192.168.1.110") ?: "192.168.1.110",
            port = prefs.getInt("port", 8080),
        )
        startPolling()
    }

    fun updateHost(host: String, port: Int) {
        _state.value = _state.value.copy(host = host, port = port)
        prefs.edit { putString("host", host).putInt("port", port) }
    }

    private fun startPolling() {
        viewModelScope.launch {
            while (true) {
                val s = _state.value
                val result = repo.fetchStats(s.host, s.port)
                result.fold(
                    onSuccess = { stats ->
                        _state.value = s.copy(stats = stats, error = null, isConnected = true)
                    },
                    onFailure = { e ->
                        _state.value = s.copy(error = e.message, isConnected = false)
                    }
                )
                delay(5000)
            }
        }
    }
}
