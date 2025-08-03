import React, { useCallback, useEffect, useRef, useState } from 'react';

import { PixelPosition } from './Pattern';
import { RgbColor, RgbColorPicker } from 'react-colorful';

type PatternEditorProps = {
    isEditing: boolean,
    pixels: number[],
    onPixelChange: (index: number, colour: number) => void
    pixelPositions: PixelPosition[],
    backgroundImageUrl: string
};

const PatternEditor : React.FC<PatternEditorProps> = ({isEditing, pixels, onPixelChange, pixelPositions, backgroundImageUrl}) => {

    const canvasRef = useRef<HTMLCanvasElement>(null);

    const loadImage = async (src: string) => 
        new Promise<HTMLImageElement>((resolve) => {
            const img = new Image();
            img.onload = () => resolve(img);
            img.src = src;
        });

    const [image, setImage] = useState(new Image());
    const [drawingColour, setDrawingColour] = useState<RgbColor>({ r: 255, g: 255, b: 255});
    const [cursorPos, setCursor] = useState({x: -1, y: -1});
    const [isDrawing, setIsDrawing] = useState(false);

    useEffect(() => {
        async function fetchData() {
            let img = await loadImage(backgroundImageUrl)
            setImage(img);
        }
        fetchData();
    }, [backgroundImageUrl]);

    useEffect(() => {
        const canvas = canvasRef.current;
        if (canvas) {
            const ctx = canvas.getContext('2d');
            if (ctx) {
                // Initialize the canvas or draw something
                ctx.globalCompositeOperation = 'source-over';
                ctx.fillStyle = '#000000ff';
                ctx.fillRect(0, 0, canvas.width, canvas.height);
                ctx.rotate(-Math.PI / 90);
                ctx.drawImage(image, 0, 75, 665, 540);
                ctx.rotate((Math.PI / 90));

                ctx.globalCompositeOperation = 'lighter';
                
                for(let i = 0; i < pixelPositions.length; i++) {

                    const x = pixelPositions[i].x * 5 + 30;
                    const y = canvas.height - pixelPositions[i].y * 5 + 4;

                    const color = pixels[i];
                    const r = (color >> 16) & 0xff;
                    const g = (color >> 8) & 0xff;
                    const b = color & 0xff;
                    const brightness =  1.6 * (0.299 * r + 0.587 * g + 0.114 * b) / 255 ;

                    const whiteFactor = Math.max(brightness - 1, 0);
                    const brightr = r + (255-r) * whiteFactor;
                    const brightg = g + (255-g) * whiteFactor;
                    const brightb = b + (255-b) * whiteFactor;
                    
                    const gradient = ctx.createRadialGradient(x, y, 0, x, y, 16);
                    gradient.addColorStop(0, `rgba(${brightr}, ${brightg}, ${brightb}, ${brightness})`);
                    gradient.addColorStop(0.5, `rgba(${r}, ${g}, ${b}, ${brightness/1.6})`);
                    gradient.addColorStop(1, `rgba(${r}, ${g}, ${b}, 0)`);

                    // Use the gradient as fill style
                    ctx.fillStyle = gradient;                    
                    ctx.beginPath();
                    ctx.ellipse(x, y , 16, 16, 0, 0, 2 * Math.PI, false);
                    ctx.fill();
                    ctx.closePath();
                }

                if(cursorPos.x >= 0 && cursorPos.y >= 0 && isEditing) {
                    ctx.beginPath();
                    ctx.fillStyle = `rgba(${drawingColour.r}, ${drawingColour.g}, ${drawingColour.b}, 0.5)`;
                    ctx.ellipse(cursorPos.x, cursorPos.y, 16, 16, 0, 0, 2 * Math.PI, false);
                    ctx.fill();
                    ctx.closePath();    
                }

            }
        }
    }, [pixels, image, cursorPos, drawingColour, isEditing, pixelPositions]);

    const drawCursor = useCallback( (event: React.MouseEvent<HTMLCanvasElement>) => {
        setCursor({x: event.nativeEvent.offsetX, y: event.nativeEvent.offsetY});

        if(isDrawing && isEditing) {
            const newColor = (drawingColour.r << 16) | (drawingColour.g << 8) | drawingColour.b;
            for(let i = 0; i < pixelPositions.length; i++) {
                const x = pixelPositions[i].x * 5 + 30;
                const y = canvasRef.current!.height - pixelPositions[i].y * 5 + 4;

                if(Math.pow(event.nativeEvent.offsetX - x, 2) + Math.pow(event.nativeEvent.offsetY - y, 2) < 100 &&
                    pixels[i] !== newColor) {
                    onPixelChange(i, newColor); // Live change the pixel color on the device too
                }
            }

        }
    }, [isEditing, isDrawing, drawingColour, pixels, onPixelChange, pixelPositions]);

    function hideCursor(): void {
        setCursor({x: -1, y: -1});
        setIsDrawing(false);
    }

    return (
        <div id="PatternEditor">
            <canvas ref={canvasRef} width={675} height={650} onMouseMove={drawCursor} onMouseLeave={hideCursor} onMouseDown={() => setIsDrawing(true)} onMouseUp={ () => setIsDrawing(false)}></canvas>
            {isEditing && <RgbColorPicker color={drawingColour} onChange={setDrawingColour}></RgbColorPicker>}
        </div>)
}

export default PatternEditor;