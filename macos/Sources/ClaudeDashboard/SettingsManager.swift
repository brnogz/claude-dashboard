import Foundation
import Observation

@Observable
final class SettingsManager {
    private let defaults = UserDefaults.standard

    var host: String {
        didSet { defaults.set(host, forKey: "host") }
    }
    var port: Int {
        didSet { defaults.set(port, forKey: "port") }
    }

    init() {
        self.host = defaults.string(forKey: "host") ?? "localhost"
        self.port = defaults.object(forKey: "port") as? Int ?? 8080
    }
}
