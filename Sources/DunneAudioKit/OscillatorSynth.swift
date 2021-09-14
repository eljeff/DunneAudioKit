//
//  OscillatorSynth.swift
//  
//
//  Created by Jeff Cooper on 9/14/21.
//
#if !os(tvOS)

import AVFoundation
import AudioKit
import AudioKitEX
import CDunneAudioKit

public enum OscSynthWaveForm: Int {
    case sinusoid
    case square
    case triangle
    case sawtooth
    case hammond

    var auValue: Float {
        return Float(rawValue)
    }
}

/// OscSynth
///
public class OscillatorSynth: Node {

    /// Connected nodes
    public var connections: [Node] { [] }

    /// Underlying AVAudioNode
    public var avAudioNode: AVAudioNode = instantiate(instrument: "osyn")

    // MARK: - Parameters

    /// Specification details for master volume
    public static let masterVolumeDef = NodeParameterDef(
        identifier: "masterVolume",
        name: "Master Volume",
        address: akGetParameterAddress("OscSynthParameterMasterVolume"),
        defaultValue: 1,
        range: 0.0 ... 1,
        unit: .generic)

    /// Master Volume (fraction)
    @Parameter(masterVolumeDef) public var masterVolume: AUValue

    /// Specification details for pitchBend
    public static let pitchBendDef = NodeParameterDef(
        identifier: "pitchBend",
        name: "Pitch bend (semitones)",
        address: akGetParameterAddress("OscSynthParameterPitchBend"),
        defaultValue: 0.0,
        range: -24 ... 24,
        unit: .relativeSemiTones)

    /// Pitch offset (semitones)
    @Parameter(pitchBendDef) public var pitchBend: AUValue

    /// Specification details for vibratoDepth
    public static let vibratoDepthDef = NodeParameterDef(
        identifier: "vibratoDepth",
        name: "Vibrato Depth",
        address: akGetParameterAddress("OscSynthParameterVibratoDepth"),
        defaultValue: 0.0,
        range: 0 ... 12,
        unit: .relativeSemiTones)

    /// Vibrato amount (semitones)
    @Parameter(vibratoDepthDef) public var vibratoDepth: AUValue

    /// Specification details for vibratoFreq
    public static let vibratoFreqDef = NodeParameterDef(
        identifier: "vibratoFreq",
        name: "Vibrato Frequency",
        address: akGetParameterAddress("OscSynthParameterVibratoFreq"),
        defaultValue: 3.0,
        range: 0.01 ... 100,
        unit: .hertz)

    /// Vibrato frequency (hertz)
    @Parameter(vibratoFreqDef) public var vibratoFreq: AUValue

    /// Specification details for filterCutoff
    public static let filterCutoffDef = NodeParameterDef(
        identifier: "filterCutoff",
        name: "Filter Cutoff",
        address: akGetParameterAddress("OscSynthParameterFilterCutoff"),
        defaultValue: 100.0,
        range: 0 ... 100,
        unit: .generic)

    /// Filter cutoff (harmonic ratio)
    @Parameter(filterCutoffDef) public var filterCutoff: AUValue

    /// Specification details for filterStrength
    public static let filterStrengthDef = NodeParameterDef(
        identifier: "filterStrength",
        name: "Filter Strength",
        address: akGetParameterAddress("OscSynthParameterFilterStrength"),
        defaultValue: 00,
        range: 0 ... 100,
        unit: .generic)

    /// filterStrength
    @Parameter(filterStrengthDef) public var filterStrength: AUValue

    /// Specification details for filterResonance
    public static let filterResonanceDef = NodeParameterDef(
        identifier: "filterResonance",
        name: "Filter Resonance",
        address: akGetParameterAddress("OscSynthParameterFilterResonance"),
        defaultValue: 0,
        range: -20 ... 20,
        unit: .generic)

    /// Filter resonance (dB)
    @Parameter(filterResonanceDef) public var filterResonance: AUValue

    /// Specification details for attackDuration
    public static let attackDurationDef = NodeParameterDef(
        identifier: "attackDuration",
        name: "Attack Duration (s)",
        address: akGetParameterAddress("OscSynthParameterAttackDuration"),
        defaultValue: 0,
        range: 0 ... 10,
        unit: .seconds)

    /// Amplitude attack duration (seconds)
    @Parameter(attackDurationDef) public var attackDuration: AUValue


    /// Specification details for decayDuration
    public static let decayDurationDef = NodeParameterDef(
        identifier: "decayDuration",
        name: "Decay Duration (s)",
        address: akGetParameterAddress("OscSynthParameterDecayDuration"),
        defaultValue: 0,
        range: 0 ... 10,
        unit: .seconds)

    /// Amplitude decay duration (seconds)
    @Parameter(decayDurationDef) public var decayDuration: AUValue

    /// Specification details for sustainLevel
    public static let sustainLevelDef = NodeParameterDef(
        identifier: "sustainLevel",
        name: "Sustain Level",
        address: akGetParameterAddress("OscSynthParameterSustainLevel"),
        defaultValue: 1,
        range: 0 ... 1,
        unit: .generic)

    /// Amplitude sustain level (fraction)
    @Parameter(sustainLevelDef) public var sustainLevel: AUValue

    /// Specification details for releaseDuration
    public static let releaseDurationDef = NodeParameterDef(
        identifier: "releaseDuration",
        name: "Release Duration (s)",
        address: akGetParameterAddress("OscSynthParameterReleaseDuration"),
        defaultValue: 0,
        range: 0 ... 10,
        unit: .seconds)

    /// Amplitude release duration (seconds)
    @Parameter(releaseDurationDef) public var releaseDuration: AUValue

    /// Specification details for filterAttackDuration
    public static let filterAttackDurationDef = NodeParameterDef(
        identifier: "filterAttackDuration",
        name: "Filter Attack Duration (s)",
        address: akGetParameterAddress("OscSynthParameterFilterAttackDuration"),
        defaultValue: 0,
        range: 0 ... 10,
        unit: .seconds)

    /// Filter Amplitude attack duration (seconds)
    @Parameter(filterAttackDurationDef) public var filterAttackDuration: AUValue


    /// Specification details for filterDecayDuration
    public static let filterDecayDurationDef = NodeParameterDef(
        identifier: "filterDecayDuration",
        name: "Filter Decay Duration (s)",
        address: akGetParameterAddress("OscSynthParameterFilterDecayDuration"),
        defaultValue: 0,
        range: 0 ... 10,
        unit: .seconds)

    /// Filter Amplitude decay duration (seconds)
    @Parameter(filterDecayDurationDef) public var filterDecayDuration: AUValue

    /// Specification details for filterSustainLevel
    public static let filterSustainLevelDef = NodeParameterDef(
        identifier: "filterSustainLevel",
        name: "Filter Sustain Level",
        address: akGetParameterAddress("OscSynthParameterFilterSustainLevel"),
        defaultValue: 1,
        range: 0 ... 1,
        unit: .generic)

    /// Filter Amplitude sustain level (fraction)
    @Parameter(filterSustainLevelDef) public var filterSustainLevel: AUValue

    /// Specification details for filterReleaseDuration
    public static let filterReleaseDurationDef = NodeParameterDef(
        identifier: "filterReleaseDuration",
        name: "Filter Release Duration (s)",
        address: akGetParameterAddress("OscSynthParameterFilterReleaseDuration"),
        defaultValue: 0,
        range: 0 ... 10,
        unit: .seconds)

    /// Filter Amplitude release duration (seconds)
    @Parameter(filterReleaseDurationDef) public var filterReleaseDuration: AUValue

    /// Specification details for oscillatorWaveform
    private static let oscillatorWaveformDef = NodeParameterDef(
        identifier: "oscillatorWaveform",
        name: "Oscillator Waveform",
        address: akGetParameterAddress("OscSynthParameterWaveform"),
        defaultValue: 0,
        range: 0 ... 4,
        unit: .indexed)

    /// Oscillator Waveform (0 = sinusoid, 1 = square, 2 = triangle, 3 = sawtooth, 4 = hammond
    @Parameter(oscillatorWaveformDef) private var oscillatorWaveform: AUValue

    // MARK: - Initialization

    /// Initialize this OscSynth node
    ///
    /// - Parameters:
    ///   - masterVolume: 0.0 - 1.0
    ///   - pitchBend: semitones, signed
    ///   - vibratoDepth: semitones, typically less than 1.0
    ///   - filterCutoff: relative to sample playback pitch, 1.0 = fundamental, 2.0 = 2nd harmonic etc
    ///   - filterStrength: same units as filterCutoff; amount filter EG adds to filterCutoff
    ///   - filterResonance: dB, -20.0 - 20.0
    ///   - attackDuration: seconds, 0.0 - 10.0
    ///   - decayDuration: seconds, 0.0 - 10.0
    ///   - sustainLevel: 0.0 - 1.0
    ///   - releaseDuration: seconds, 0.0 - 10.0
    ///   - filterEnable: true to enable per-voice filters
    ///   - filterAttackDuration: seconds, 0.0 - 10.0
    ///   - filterDecayDuration: seconds, 0.0 - 10.0
    ///   - filterSustainLevel: 0.0 - 1.0
    ///   - filterReleaseDuration: seconds, 0.0 - 10.0
    ///
    public init(
        masterVolume: AUValue = masterVolumeDef.defaultValue,
        waveform: OscSynthWaveForm = .sinusoid,
        pitchBend: AUValue = pitchBendDef.defaultValue,
        vibratoDepth: AUValue = vibratoDepthDef.defaultValue,
        filterCutoff: AUValue = filterCutoffDef.defaultValue,
        filterStrength: AUValue = filterStrengthDef.defaultValue,
        filterResonance: AUValue = filterResonanceDef.defaultValue,
        attackDuration: AUValue = attackDurationDef.defaultValue,
        decayDuration: AUValue = decayDurationDef.defaultValue,
        sustainLevel: AUValue = sustainLevelDef.defaultValue,
        releaseDuration: AUValue = releaseDurationDef.defaultValue,
        filterEnable: Bool = false,
        filterAttackDuration: AUValue = filterAttackDurationDef.defaultValue,
        filterDecayDuration: AUValue = filterDecayDurationDef.defaultValue,
        filterSustainLevel: AUValue = filterSustainLevelDef.defaultValue,
        filterReleaseDuration: AUValue = filterReleaseDurationDef.defaultValue
    ) {

        setupParameters()

        self.masterVolume = masterVolume
        self.oscillatorWaveform = waveform.auValue
        self.pitchBend = pitchBend
        self.vibratoDepth = vibratoDepth
        self.filterCutoff = filterCutoff
        self.filterStrength = filterStrength
        self.filterResonance = filterResonance
        self.attackDuration = attackDuration
        self.decayDuration = decayDuration
        self.sustainLevel = sustainLevel
        self.releaseDuration = releaseDuration
        self.filterAttackDuration = filterAttackDuration
        self.filterDecayDuration = filterDecayDuration
        self.filterSustainLevel = filterSustainLevel
        self.filterReleaseDuration = filterReleaseDuration

    }

    /// Play a note on the OscSynth
    /// - Parameters:
    ///   - noteNumber: MIDI Note Number
    ///   - velocity: MIDI Velocity
    ///   - channel: MIDI Channel
    public func play(noteNumber: MIDINoteNumber,
                     velocity: MIDIVelocity,
                     channel: MIDIChannel = 0) {
        scheduleMIDIEvent(event: MIDIEvent(noteOn: noteNumber, velocity: velocity, channel: channel))
    }

    /// Stop a note
    /// - Parameters:
    ///   - noteNumber: MIDI Note Number
    ///   - channel: MIDI Channel
    public func stop(noteNumber: MIDINoteNumber, channel: MIDIChannel = 0) {
        scheduleMIDIEvent(event: MIDIEvent(noteOff: noteNumber, velocity: 0, channel: channel))
    }
}
#endif

