import Foundation
import Observation

@Observable
final class DashboardViewModel {
    var stats: StatsResponse?
    var error: String?
    var isConnected: Bool = false
    var showSettings: Bool = false

    let settings: SettingsManager
    private let service = StatsService()

    init(settings: SettingsManager = SettingsManager()) {
        self.settings = settings
        startPolling()
    }

    func startPolling() {
        service.startPolling(host: settings.host, port: settings.port) { [weak self] result in
            guard let self else { return }
            switch result {
            case .success(let stats):
                self.stats = stats
                self.error = nil
                self.isConnected = true
            case .failure(let err):
                self.error = err.localizedDescription
                self.isConnected = false
            }
        }
    }

    func restartPolling() {
        service.stopPolling()
        startPolling()
    }
}
