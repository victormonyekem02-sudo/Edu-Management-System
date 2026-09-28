import { useState } from 'react';

const markingData = [
  { phase: 'Phase 1: Planning', marks: 10, color: 'bg-blue-500', details: 'Feasibility (4), WBS/Schedule (3), Risk (3)' },
  { phase: 'Phase 2: Requirements', marks: 14, color: 'bg-cyan-500', details: 'Stakeholder analysis (3), SRS (5), Use cases (6)' },
  { phase: 'Phase 3: Design', marks: 24, color: 'bg-purple-500', details: 'Architecture (5), Class diagram (7), Behavioural (7), Database (5)' },
  { phase: 'Phase 4: Implementation', marks: 21, color: 'bg-green-500', details: 'Functionality (9), Code quality (7), Persistence/Security (5)' },
  { phase: 'Phase 5: V&V', marks: 12, color: 'bg-amber-500', details: 'Test strategy (3), Automated tests (5), System/Acceptance (4)' },
  { phase: 'Phase 6: CM & QA', marks: 6, color: 'bg-orange-500', details: 'Config. management (3), Quality assurance (3)' },
  { phase: 'Phase 7: Deployment', marks: 4, color: 'bg-rose-500', details: 'Deployment diagram, guides, manuals, release notes' },
  { phase: 'Phase 8: Maintenance', marks: 6, color: 'bg-pink-500', details: 'Classification/Impact (2), Implementation (2), Regression (2)' },
  { phase: 'Professional Issues', marks: 3, color: 'bg-indigo-500', details: 'Data protection, security, accessibility, fairness' },
];

const gradeDescriptors = [
  { range: '80–100%', label: 'Outstanding', color: 'text-emerald-400', bg: 'bg-emerald-500/10 border-emerald-500/30', desc: 'Complete, precise, professionally presented. Decisions critically justified. Artefacts fully consistent and traceable.' },
  { range: '70–79%', label: 'Very Good', color: 'text-blue-400', bg: 'bg-blue-500/10 border-blue-500/30', desc: 'Complete and accurate with clear justification. Strong consistency across artefacts. Minor weaknesses only.' },
  { range: '60–69%', label: 'Good', color: 'text-cyan-400', bg: 'bg-cyan-500/10 border-cyan-500/30', desc: 'Mostly complete and correct with some justification. Some inconsistencies or gaps in depth.' },
  { range: '50–59%', label: 'Satisfactory', color: 'text-amber-400', bg: 'bg-amber-500/10 border-amber-500/30', desc: 'Meets basic requirements but largely descriptive. Limited justification, notational errors or inconsistencies.' },
  { range: '40–49%', label: 'Weak', color: 'text-orange-400', bg: 'bg-orange-500/10 border-orange-500/30', desc: 'Significant omissions or errors. Weak links between artefacts. Little justification.' },
  { range: '< 40%', label: 'Fail', color: 'text-red-400', bg: 'bg-red-500/10 border-red-500/30', desc: 'Largely incomplete, incorrect or inconsistent with the case study.' },
];

export default function MarkingScheme() {
  const [selectedPhase, setSelectedPhase] = useState<number | null>(null);
  const totalMarks = 100;

  return (
    <section id="marks" className="py-16 lg:py-24 bg-white/[0.02]">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="text-center mb-12">
          <h2 className="text-3xl sm:text-4xl font-bold text-white mb-3">Marking Scheme</h2>
          <p className="text-slate-400 max-w-2xl mx-auto">
            100 marks across 9 components. Higher marks require justified decisions and critical insight.
          </p>
        </div>

        <div className="grid lg:grid-cols-3 gap-8">
          {/* Bar chart */}
          <div className="lg:col-span-2 bg-white/5 backdrop-blur-sm rounded-2xl border border-white/10 p-6">
            <h3 className="text-lg font-semibold text-white mb-6">Mark Distribution</h3>
            <div className="space-y-4">
              {markingData.map((item, index) => (
                <button
                  key={item.phase}
                  onClick={() => setSelectedPhase(selectedPhase === index ? null : index)}
                  className="w-full text-left group"
                >
                  <div className="flex items-center justify-between mb-1.5">
                    <span className="text-sm text-slate-300 group-hover:text-white transition-colors">{item.phase}</span>
                    <span className="text-sm font-bold text-white">{item.marks}</span>
                  </div>
                  <div className="h-8 bg-white/5 rounded-lg overflow-hidden relative">
                    <div
                      className={`h-full ${item.color} rounded-lg transition-all duration-500 flex items-center justify-end pr-3 ${
                        selectedPhase === index ? 'opacity-100' : 'opacity-80 group-hover:opacity-100'
                      }`}
                      style={{ width: `${(item.marks / totalMarks) * 100}%` }}
                    >
                      <span className="text-xs font-bold text-white/90">{item.marks}%</span>
                    </div>
                  </div>
                  {selectedPhase === index && (
                    <p className="mt-2 text-xs text-slate-400 pl-2">{item.details}</p>
                  )}
                </button>
              ))}
            </div>
          </div>

          {/* Grade descriptors */}
          <div className="bg-white/5 backdrop-blur-sm rounded-2xl border border-white/10 p-6">
            <h3 className="text-lg font-semibold text-white mb-6">Assessment Levels</h3>
            <div className="space-y-3">
              {gradeDescriptors.map((grade) => (
                <div key={grade.label} className={`rounded-lg border p-3 ${grade.bg}`}>
                  <div className="flex items-center justify-between mb-1">
                    <span className={`font-bold text-sm ${grade.color}`}>{grade.label}</span>
                    <span className="text-xs text-slate-400">{grade.range}</span>
                  </div>
                  <p className="text-xs text-slate-400 leading-relaxed">{grade.desc}</p>
                </div>
              ))}
            </div>
          </div>
        </div>

        {/* Report structure */}
        <div className="mt-8 bg-white/5 backdrop-blur-sm rounded-2xl border border-white/10 p-6">
          <h3 className="text-lg font-semibold text-white mb-4">📄 Report Structure (6,000–8,000 words)</h3>
          <div className="grid sm:grid-cols-2 lg:grid-cols-3 gap-2">
            {[
              '1. Title page, abstract & TOC',
              '2. Introduction',
              '3. Project management plan',
              '4. Requirements engineering',
              '5. Software design document',
              '6. Implementation',
              '7. Verification & validation',
              '8. Config. management & QA',
              '9. Deployment',
              '10. Maintenance (CR1)',
              '11. Professional issues',
              '12. Conclusion',
              '13. References',
              '14. Appendices',
            ].map((item) => (
              <div key={item} className="flex items-center gap-2 px-3 py-2 rounded-lg bg-white/5 text-sm text-slate-300">
                <span className="w-1.5 h-1.5 rounded-full bg-indigo-400" />
                {item}
              </div>
            ))}
          </div>
        </div>
      </div>
    </section>
  );
}
