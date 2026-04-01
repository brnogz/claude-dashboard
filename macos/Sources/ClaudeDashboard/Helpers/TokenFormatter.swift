import Foundation

func formatTokens(_ tokens: Int) -> String {
    switch tokens {
    case 1_000_000_000...: String(format: "%.1fB", Double(tokens) / 1_000_000_000)
    case 1_000_000...:     String(format: "%.1fM", Double(tokens) / 1_000_000)
    case 1_000...:         String(format: "%.1fK", Double(tokens) / 1_000)
    default:               "\(tokens)"
    }
}

func formatDuration(_ seconds: Int) -> String {
    if seconds >= 3600 {
        "\(seconds / 3600)h \(String(format: "%02d", (seconds % 3600) / 60))m"
    } else {
        "\(seconds / 60)m"
    }
}
