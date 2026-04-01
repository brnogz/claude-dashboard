package com.claude.dashboard.data

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import okhttp3.OkHttpClient
import okhttp3.Request
import java.util.concurrent.TimeUnit

class DashboardRepository {

    private val client = OkHttpClient.Builder()
        .connectTimeout(5, TimeUnit.SECONDS)
        .readTimeout(5, TimeUnit.SECONDS)
        .build()

    private val json = Json { ignoreUnknownKeys = true }

    suspend fun fetchStats(host: String, port: Int): Result<StatsResponse> = withContext(Dispatchers.IO) {
        try {
            val request = Request.Builder()
                .url("http://$host:$port/stats")
                .build()
            val response = client.newCall(request).execute()
            val body = response.body?.string() ?: return@withContext Result.failure(Exception("Empty response"))
            if (!response.isSuccessful) return@withContext Result.failure(Exception("HTTP ${response.code}"))
            val stats = json.decodeFromString<StatsResponse>(body)
            Result.success(stats)
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
}
