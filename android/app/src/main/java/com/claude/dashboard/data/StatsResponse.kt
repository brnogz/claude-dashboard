package com.claude.dashboard.data

import kotlinx.serialization.Serializable

@Serializable
data class StatsResponse(
    val sessions: Sessions,
    val current: CurrentSession,
    val today: PeriodData,
    val week: WeekData,
    val usage: UsageData,
)

@Serializable
data class Sessions(val active: Int)

@Serializable
data class CurrentSession(
    val id: String,
    val inputTokens: Long,
    val outputTokens: Long,
    val cacheRead: Long,
    val cacheCreate: Long,
    val messages: Int,
    val duration: Long,
) {
    val totalTokens: Long get() = inputTokens + outputTokens + cacheRead + cacheCreate
}

@Serializable
data class PeriodData(
    val inputTokens: Long,
    val outputTokens: Long,
    val cacheRead: Long,
    val cacheCreate: Long,
    val messages: Int,
) {
    val totalTokens: Long get() = inputTokens + outputTokens + cacheRead + cacheCreate
}

@Serializable
data class WeekData(
    val inputTokens: Long,
    val outputTokens: Long,
    val cacheRead: Long,
    val cacheCreate: Long,
    val messages: Int,
    val daily: List<Long>,
    val labels: List<String>,
) {
    val totalTokens: Long get() = inputTokens + outputTokens + cacheRead + cacheCreate
}

@Serializable
data class UsageData(
    val pct: Int,
    val plan: String,
    val window: String = "5h",
    val used: Int = 0,
    val limit: Int = 0,
)
