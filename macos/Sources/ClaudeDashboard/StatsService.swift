import Foundation

final class StatsService {
    private var timer: Timer?

    func startPolling(host: String, port: Int, onResult: @escaping (Result<StatsResponse, Error>) -> Void) {
        stopPolling()
        // Fetch immediately, then every 5 seconds
        fetch(host: host, port: port, completion: onResult)
        timer = Timer.scheduledTimer(withTimeInterval: 5.0, repeats: true) { [weak self] _ in
            self?.fetch(host: host, port: port, completion: onResult)
        }
    }

    func stopPolling() {
        timer?.invalidate()
        timer = nil
    }

    private func fetch(host: String, port: Int, completion: @escaping (Result<StatsResponse, Error>) -> Void) {
        guard let url = URL(string: "http://\(host):\(port)/stats") else { return }
        var request = URLRequest(url: url)
        request.timeoutInterval = 5

        URLSession.shared.dataTask(with: request) { data, _, error in
            DispatchQueue.main.async {
                if let error {
                    completion(.failure(error))
                    return
                }
                guard let data else {
                    completion(.failure(URLError(.badServerResponse)))
                    return
                }
                do {
                    let stats = try JSONDecoder().decode(StatsResponse.self, from: data)
                    completion(.success(stats))
                } catch {
                    completion(.failure(error))
                }
            }
        }.resume()
    }
}
