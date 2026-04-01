import AppKit
import SwiftUI

@main
struct ClaudeDashboardApp {
    static func main() {
        let app = NSApplication.shared
        let delegate = AppDelegate()
        app.delegate = delegate
        app.setActivationPolicy(.accessory)
        app.run()
    }
}

class AppDelegate: NSObject, NSApplicationDelegate {
    private var statusItem: NSStatusItem!
    private var popover: NSPopover!
    private let viewModel = DashboardViewModel()

    func applicationDidFinishLaunching(_ notification: Notification) {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)

        if let button = statusItem.button {
            button.image = makeClaudeIcon()
            button.imagePosition = .imageLeading
            button.title = " --%"
            button.action = #selector(togglePopover)
            button.target = self
        }

        popover = NSPopover()
        popover.contentSize = NSSize(width: 320, height: 480)
        popover.behavior = .transient
        popover.animates = true
        popover.contentViewController = NSHostingController(rootView: DashboardView(viewModel: viewModel))

        // Start observing stats to update the status bar icon
        startStatusBarUpdater()
    }

    @objc private func togglePopover(_ sender: Any?) {
        guard let button = statusItem.button else { return }
        if popover.isShown {
            popover.performClose(sender)
        } else {
            popover.show(relativeTo: button.bounds, of: button, preferredEdge: .minY)
            NSApp.activate(ignoringOtherApps: true)
        }
    }

    private func startStatusBarUpdater() {
        Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { [weak self] _ in
            self?.updateStatusBarIcon()
        }
    }

    private func updateStatusBarIcon() {
        guard let button = statusItem.button else { return }

        if let stats = viewModel.stats {
            let pct = stats.usage.pct
            let color: NSColor
            switch pct {
            case 81...: color = .systemRed
            case 51...80: color = .systemYellow
            default: color = .systemGreen
            }
            button.attributedTitle = NSAttributedString(
                string: " \(pct)%",
                attributes: [.foregroundColor: color]
            )
        } else if !viewModel.isConnected {
            button.attributedTitle = NSAttributedString(
                string: " --",
                attributes: [.foregroundColor: NSColor.systemGray]
            )
        }
    }

    /// Draws the Claude spark logo as a monochrome template image for the menu bar.
    /// Separated oval petals around a center dot — matches the real Claude icon.
    private func makeClaudeIcon() -> NSImage {
        let px = 36 // @2x for retina
        let rep = NSBitmapImageRep(
            bitmapDataPlanes: nil,
            pixelsWide: px, pixelsHigh: px,
            bitsPerSample: 8, samplesPerPixel: 4,
            hasAlpha: true, isPlanar: false,
            colorSpaceName: .deviceRGB,
            bytesPerRow: 0, bitsPerPixel: 0
        )!

        let ctx = NSGraphicsContext(bitmapImageRep: rep)!
        NSGraphicsContext.saveGraphicsState()
        NSGraphicsContext.current = ctx

        let cx: CGFloat = CGFloat(px) / 2
        let cy: CGFloat = CGFloat(px) / 2

        NSColor.black.setFill()

        // Center dot
        let dotR: CGFloat = 2.8
        NSBezierPath(ovalIn: NSRect(x: cx - dotR, y: cy - dotR,
                                     width: dotR * 2, height: dotR * 2)).fill()

        // Petals: each is an oval placed along a radial direction, NOT touching center.
        // Varying distance, length, and width for organic feel.
        struct Petal {
            let angle: CGFloat    // direction from center
            let start: CGFloat    // inner edge distance from center
            let length: CGFloat   // how long the oval is
            let width: CGFloat    // how wide the oval is
        }

        let petals: [Petal] = [
            Petal(angle: -1.57,  start: 5.0, length: 11.0, width: 4.0),  // top
            Petal(angle: -0.25,  start: 4.8, length:  8.5, width: 3.6),  // upper right
            Petal(angle:  1.05,  start: 4.8, length:  9.5, width: 3.8),  // lower right
            Petal(angle:  2.20,  start: 4.8, length:  8.0, width: 3.6),  // lower left
            Petal(angle:  3.60,  start: 4.8, length:  9.0, width: 3.8),  // upper left
        ]

        for p in petals {
            let ovalCx = cx + cos(p.angle) * (p.start + p.length / 2)
            let ovalCy = cy + sin(p.angle) * (p.start + p.length / 2)

            let transform = NSAffineTransform()
            transform.translateX(by: ovalCx, yBy: ovalCy)
            transform.rotate(byRadians: p.angle)

            let oval = NSBezierPath(ovalIn: NSRect(
                x: -p.length / 2, y: -p.width / 2,
                width: p.length, height: p.width
            ))
            oval.transform(using: transform as AffineTransform)
            oval.fill()
        }

        NSGraphicsContext.restoreGraphicsState()

        let image = NSImage(size: NSSize(width: CGFloat(px) / 2, height: CGFloat(px) / 2))
        image.addRepresentation(rep)
        image.isTemplate = true
        return image
    }
}
