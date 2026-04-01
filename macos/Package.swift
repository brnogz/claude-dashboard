// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "ClaudeDashboard",
    platforms: [.macOS(.v14)],
    targets: [
        .executableTarget(
            name: "ClaudeDashboard",
            path: "Sources/ClaudeDashboard"
        ),
    ]
)
