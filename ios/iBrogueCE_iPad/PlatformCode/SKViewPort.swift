//
//  SKViewPort.swift
//  iBrogue_iPad
//
//  This file is part of Brogue.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

//import UIKit
import SpriteKit

class SKViewPort: SKView {  
    var rogueScene: RogueScene!
    
    required init?(coder aDecoder: NSCoder) {
        let rect = UIScreen.main.bounds
        // go max retina on initial size or scaling of text is ugly
        let scale = UIScreen.main.scale
        rogueScene = RogueScene(size: CGSize(width: rect.size.width * scale, height: rect.size.height * scale), rows: 34, cols: 100)
        rogueScene.scaleMode = .fill
        super.init(coder: aDecoder)
        
       // showsFPS = true
      //  showsNodeCount = true
        ignoresSiblingOrder = true
        backgroundColor = .black
    }
    
    override func awakeFromNib() {
        presentScene(rogueScene)
    }
    
    @objc public func setCell(x: Int, y: Int, code: UInt32, bgColor: CGColor, fgColor: CGColor) {
        rogueScene.setCell(x: x, y: y, code: code, bgColor: bgColor, fgColor: fgColor)
    }
}
