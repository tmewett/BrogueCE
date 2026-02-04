//
//  Cell.swift
//  Brogue
//


import SpriteKit

class MagCell: Cell {
    convenience init(cell: Cell, magnify: CGFloat) {
        self.init(x: cell.position.x, y: cell.position.y, size: CGSize(width: cell.size.width * (magnify), height: cell.size.height * (magnify)))
        fgcolor = cell.fgcolor
        bgcolor = cell.bgcolor
        
        if cell.glyph != nil {
            glyph = cell.glyph
        }
    }
}

class Cell {
    let foreground: SKSpriteNode
    let background: SKSpriteNode
    var size: CGSize {
        get {
            return foreground.size
        }
        set(newSize) {
            foreground.size = newSize
            background.size = newSize
        }
    }
    
    var position: CGPoint {
        get {
            return foreground.position
        }
        set(newPosition) {
            foreground.position = newPosition
            background.position = newPosition
        }
    }
    
    var glyph: SKTexture? {
        set(newGlyph) {
            foreground.texture = newGlyph
        }
        get {
            return foreground.texture
        }
    }
    
    var fgcolor: SKColor {
        set(newColor) {
            foreground.color = newColor
        }
        get {
            return foreground.color
        }
    }
    
    var bgcolor: SKColor {
        set(newColor) {
            background.color = newColor
        }
        get {
            return background.color
        }
    }
    
    init(x: CGFloat, y: CGFloat, size: CGSize) {
        foreground = SKSpriteNode(color: .clear, size: size)
        background = SKSpriteNode(color: .black, size: size)
        
        // Allow colours to be changed by blending their white components
        foreground.colorBlendFactor = 1
        background.colorBlendFactor = 1
        
        // The positions should be static
        position = CGPoint(x: x, y: y)
        foreground.zPosition = 1 // Foreground
        
        background.anchorPoint = CGPoint(x: 0, y: 0)
        foreground.anchorPoint = CGPoint(x: 0, y: 0)
    }
}
