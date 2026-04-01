import Foundation

struct StatsResponse: Codable {
    let sessions: Sessions
    let current: CurrentSession
    let today: PeriodData
    let week: WeekData
    let usage: UsageData
}

struct Sessions: Codable {
    let active: Int
}

struct CurrentSession: Codable {
    let id: String
    let inputTokens: Int
    let outputTokens: Int
    let cacheRead: Int
    let cacheCreate: Int
    let messages: Int
    let duration: Int

    var totalTokens: Int { inputTokens + outputTokens + cacheRead + cacheCreate }
}

struct PeriodData: Codable {
    let inputTokens: Int
    let outputTokens: Int
    let cacheRead: Int
    let cacheCreate: Int
    let messages: Int

    var totalTokens: Int { inputTokens + outputTokens + cacheRead + cacheCreate }
}

struct WeekData: Codable {
    let inputTokens: Int
    let outputTokens: Int
    let cacheRead: Int
    let cacheCreate: Int
    let messages: Int
    let daily: [Int]
    let labels: [String]

    var totalTokens: Int { inputTokens + outputTokens + cacheRead + cacheCreate }
}

struct UsageData: Codable {
    let pct: Int
    let plan: String
    let window: String
    let used: Int
    let limit: Int
}
