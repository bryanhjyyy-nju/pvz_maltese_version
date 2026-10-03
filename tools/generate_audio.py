"""Generate original, dependency-free PCM music and effects for the game.

Run from any directory with Python 3. Uses only the standard library.
"""
import math
import struct
import wave
from pathlib import Path

RATE = 22050
OUT = Path(__file__).resolve().parents[1] / "Media"


def note(buffer, start, duration, midi, volume=0.2):
    frequency = 440 * 2 ** ((midi - 69) / 12)
    length = int(duration * RATE)
    for i in range(length):
        position = int(start * RATE) + i
        if position >= len(buffer):
            break
        t = i / RATE
        envelope = min(1, t / 0.012) * (1 - i / length) ** 2
        tone = math.sin(2 * math.pi * frequency * t)
        tone += 0.18 * math.sin(4 * math.pi * frequency * t)
        buffer[position] += volume * envelope * tone


def save(name, buffer):
    with wave.open(str(OUT / (name + ".wav")), "wb") as output:
        output.setparams((1, 2, RATE, 0, "NONE", "not compressed"))
        output.writeframes(b"".join(struct.pack("<h", int(max(-1, min(1, v)) * 30000)) for v in buffer))


def music(name, beat, transpose):
    buffer = [0.0] * int(32 * beat * RATE)
    melody = [72,76,79,76,74,77,81,77,71,74,79,74,72,76,79,83,
              81,79,76,72,77,76,74,71,72,74,76,79,76,74,72,67]
    for i, pitch in enumerate(melody):
        note(buffer,i*beat,beat*.85,pitch+transpose,.17)
        if i%2 == 0:
            note(buffer,i*beat,beat*1.8,[48,53,55,48][i//8]+transpose,.13)
        note(buffer,i*beat,beat*.2,36+transpose,.07)
    save(name,buffer)


if __name__ == "__main__":
    OUT.mkdir(exist_ok=True)
    music("menu",.45,0)
    music("battle",.32,-5)
    effects = {"pause":[79,72,67], "click":[84], "plant":[55,62], "collect":[79,84,88],
               "shoot":[79,67], "hit":[43,38], "shovel":[50,43],
               "win":[72,76,79,84], "lose":[60,56,53,48]}
    for name, pitches in effects.items():
        step = .18 if name in ("win","lose") else .06
        buffer = [0.0] * int((len(pitches)*step+.20)*RATE)
        for i,pitch in enumerate(pitches):
            note(buffer,i*step,step+.15,pitch,.28)
        save(name,buffer)
