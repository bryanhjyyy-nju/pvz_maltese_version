"""Generate original, dependency-free PCM music and effects for the game.

Run from any directory with Python 3. Uses only the standard library.
"""
import math
import struct
import wave
import random
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


def texture(name, duration, frequency, noise, seed):
    """Decaying noise gives physical actions a timbre distinct from the music."""
    rng = random.Random(seed)
    buffer = []
    smoothed = 0.0
    length = int(duration * RATE)
    for i in range(length):
        t = i / RATE
        smoothed = smoothed * .65 + rng.uniform(-1,1) * .35
        envelope = min(1,t/.005) * (1-i/length)**2
        tone = math.sin(2*math.pi*frequency*t*(1-.35*i/length))
        if name == "bite":
            envelope *= .35 + .65 * abs(math.sin(2*math.pi*7*t))
        buffer.append(.65*envelope*((1-noise)*tone+noise*smoothed))
    save(name,buffer)


def announcement(name, seed):
    """A descending bass impact, noisy transient and short brass-like chord."""
    rng = random.Random(seed)
    duration = 1.3
    buffer = [0.0] * int(duration * RATE)
    for i in range(len(buffer)):
        t = i / RATE
        bass = math.sin(2 * math.pi * (80 * t - 18 * t * t)) * math.exp(-4 * t)
        transient = rng.uniform(-1, 1) * math.exp(-24 * t)
        buffer[i] = .55 * bass + .2 * transient
    for pitch in ([48,55,60,64] if name == "readyImpact" else [43,50,55,59]):
        note(buffer,.08,.95,pitch,.12)
    save(name,buffer)


def victory():
    buffer = [0.0] * int(3.6 * RATE)
    melody = [72, 72, 76, 79, 84, 79, 84, 88, 84]
    for i, pitch in enumerate(melody):
        duration = .55 if i < 8 else 1.35
        note(buffer, i * .25, duration, pitch, .25)
        note(buffer, i * .25, duration, pitch - 12, .12)
    for pitch in [48, 55, 60, 64]:
        note(buffer, 2.0, 1.5, pitch, .10)
    save("win", buffer)


if __name__ == "__main__":
    OUT.mkdir(exist_ok=True)
    music("menu",.45,0)
    music("battle",.32,-5)
    effects = {"pause":[79,72,67], "shovelPickup":[72,84], "shovelPutdown":[64,52],
               "death":[67,60,48], "guitarShot":[64,71,76], "guitarHit":[83,71],
               "click":[84], "plant":[55,62], "collect":[79,84,88],
               "shoot":[79,67], "hit":[43,38], "shovel":[50,43],
               "win":[72,76,79,84], "lose":[60,56,53,48]}
    for name, pitches in effects.items():
        step = .18 if name in ("win","lose") else .06
        buffer = [0.0] * int((len(pitches)*step+.20)*RATE)
        for i,pitch in enumerate(pitches):
            note(buffer,i*step,step+.15,pitch,.28)
        save(name,buffer)
    texture("bite",.24,105,.88,11)
    texture("hit",.15,155,.5,12)
    texture("uproot",.32,80,.95,13)
    texture("plant",.19,95,.72,14)
    announcement("readyImpact",15)
    announcement("finalWave",16)
    victory()
