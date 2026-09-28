import { useState } from 'react';

interface ChecklistItem {
  id: string;
  text: string;
  phase: string;
}

const deliverables: ChecklistItem[] = [
  // Phase 1
  { id: 'd1', text: 'Feasibility study (technical, operational, economic, legal, schedule)', phase: 'Phase 1' },
  { id: 'd2', text: 'Work Breakdown Structure', phase: 'Phase 1' },
  { id: 'd3', text: 'Gantt chart with dependencies, milestones, critical path', phase: 'Phase 1' },
  { id: 'd4', text: 'Risk register (6+ risks with likelihood, impact, exposure, mitigation, owner)', phase: 'Phase 1' },
  // Phase 2
  { id: 'd5', text: 'Stakeholder analysis with conflict resolution', phase: 'Phase 2' },
  { id: 'd6', text: 'SRS with 12+ functional & 6+ non-functional requirements (MoSCoW)', phase: 'Phase 2' },
  { id: 'd7', text: 'Use Case Diagram (all 4 actors, include/extend/generalisation)', phase: 'Phase 2' },
  { id: 'd8', text: '3 fully dressed use case specifications (incl. "Submit assignment")', phase: 'Phase 2' },
  // Phase 3
  { id: 'd9', text: 'Architecture selection with justification & component diagram', phase: 'Phase 3' },
  { id: 'd10', text: 'Design-level Class Diagram (visibility, types, multiplicities, inheritance)', phase: 'Phase 3' },
  { id: 'd11', text: '2+ design patterns applied with explanation (SOLID principles)', phase: 'Phase 3' },
  { id: 'd12', text: 'State Machine Diagram for Submission lifecycle', phase: 'Phase 3' },
  { id: 'd13', text: 'Activity Diagram with swimlanes (submission → released mark)', phase: 'Phase 3' },
  { id: 'd14', text: 'Sequence Diagrams: "Submit assignment" & "Moderate and release marks"', phase: 'Phase 3' },
  { id: 'd15', text: 'ERD normalised to 3NF + data dictionary + SQL schema', phase: 'Phase 3' },
  // Phase 4
  { id: 'd16', text: 'C++17 implementation with role-based access (4 user types)', phase: 'Phase 4' },
  { id: 'd17', text: 'Late-penalty, extension, resubmission & moderation rules', phase: 'Phase 4' },
  { id: 'd18', text: 'Abstract User class with inheritance & polymorphism', phase: 'Phase 4' },
  { id: 'd19', text: 'SQLite persistence via data-access layer (parameterised queries)', phase: 'Phase 4' },
  { id: 'd20', text: 'Salted password hashing (recognised library)', phase: 'Phase 4' },
  { id: 'd21', text: 'Audit log for all mark changes', phase: 'Phase 4' },
  { id: 'd22', text: 'Exception handling & input validation', phase: 'Phase 4' },
  { id: 'd23', text: 'CMake build system + coding standard', phase: 'Phase 4' },
  // Phase 5
  { id: 'd24', text: 'Test plan (V-Model mapping, entry/exit criteria, EP & BVA)', phase: 'Phase 5' },
  { id: 'd25', text: 'Automated unit & integration tests (GoogleTest/Catch2)', phase: 'Phase 5' },
  { id: 'd26', text: '70%+ line coverage report for business-logic layer', phase: 'Phase 5' },
  { id: 'd27', text: 'System & acceptance tests with defect log', phase: 'Phase 5' },
  // Phase 6
  { id: 'd28', text: 'Git branching strategy documentation', phase: 'Phase 6' },
  { id: 'd29', text: 'CI pipeline (build + test on every push)', phase: 'Phase 6' },
  { id: 'd30', text: 'Static analysis report (cppcheck/clang-tidy)', phase: 'Phase 6' },
  { id: 'd31', text: 'Code metrics & self code review record', phase: 'Phase 6' },
  // Phase 7
  { id: 'd32', text: 'UML Deployment Diagram', phase: 'Phase 7' },
  { id: 'd33', text: 'Installation guide', phase: 'Phase 7' },
  { id: 'd34', text: 'User manuals (one per role)', phase: 'Phase 7' },
  { id: 'd35', text: 'Release notes (tagged v1.0)', phase: 'Phase 7' },
  // Phase 8
  { id: 'd36', text: 'CR1 classification & impact analysis', phase: 'Phase 8' },
  { id: 'd37', text: 'CR1 implementation as v1.1', phase: 'Phase 8' },
  { id: 'd38', text: 'Regression test results', phase: 'Phase 8' },
  // Final
  { id: 'd39', text: 'Professional, legal & ethical issues discussion', phase: 'Final' },
  { id: 'd40', text: 'Final report (6,000–8,000 words, PDF)', phase: 'Final' },
  { id: 'd41', text: 'Requirements traceability matrix', phase: 'Final' },
  { id: 'd42', text: 'Updated risk register', phase: 'Final' },
];

const phases = ['Phase 1', 'Phase 2', 'Phase 3', 'Phase 4', 'Phase 5', 'Phase 6', 'Phase 7', 'Phase 8', 'Final'];

export default function Deliverables() {
  const [completed, setCompleted] = useState<Set<string>>(new Set());

  const toggle = (id: string) => {
    setCompleted((prev) => {
      const next = new Set(prev);
      if (next.has(id)) next.delete(id);
      else next.add(id);
      return next;
    });
  };

  const totalItems = deliverables.length;
  const completedCount = completed.size;
  const progress = Math.round((completedCount / totalItems) * 100);

  return (
    <section id="deliverables" className="py-16 lg:py-24">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="text-center mb-12">
          <h2 className="text-3xl sm:text-4xl font-bold text-white mb-3">Deliverables Checklist</h2>
          <p className="text-slate-400 max-w-2xl mx-auto">
            Track your progress across all phases. Click items to mark them as completed.
          </p>
        </div>

        {/* Progress bar */}
        <div className="mb-8 bg-white/5 backdrop-blur-sm rounded-2xl border border-white/10 p-6">
          <div className="flex items-center justify-between mb-3">
            <span className="text-sm font-medium text-slate-300">Overall Progress</span>
            <span className="text-sm font-bold text-white">{completedCount}/{totalItems} ({progress}%)</span>
          </div>
          <div className="h-4 bg-white/10 rounded-full overflow-hidden">
            <div
              className="h-full bg-gradient-to-r from-indigo-500 to-purple-500 rounded-full transition-all duration-500"
              style={{ width: `${progress}%` }}
            />
          </div>
          {progress === 100 && (
            <p className="text-center text-green-400 text-sm mt-3 font-medium">🎉 All deliverables complete!</p>
          )}
        </div>

        {/* Phase columns */}
        <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-4">
          {phases.map((phase) => {
            const items = deliverables.filter((d) => d.phase === phase);
            const phaseCompleted = items.filter((d) => completed.has(d.id)).length;
            return (
              <div key={phase} className="bg-white/5 backdrop-blur-sm rounded-xl border border-white/10 p-4">
                <div className="flex items-center justify-between mb-3">
                  <h3 className="font-bold text-white text-sm">{phase}</h3>
                  <span className={`text-xs font-medium px-2 py-1 rounded-full ${
                    phaseCompleted === items.length
                      ? 'bg-green-500/20 text-green-300'
                      : 'bg-white/10 text-slate-400'
                  }`}>
                    {phaseCompleted}/{items.length}
                  </span>
                </div>
                <div className="space-y-2">
                  {items.map((item) => (
                    <label
                      key={item.id}
                      className={`flex items-start gap-2 p-2 rounded-lg cursor-pointer transition-all ${
                        completed.has(item.id) ? 'bg-green-500/10' : 'hover:bg-white/5'
                      }`}
                    >
                      <input
                        type="checkbox"
                        checked={completed.has(item.id)}
                        onChange={() => toggle(item.id)}
                        className="mt-0.5 w-4 h-4 rounded border-white/20 bg-white/10 text-indigo-500 focus:ring-indigo-500 focus:ring-offset-0"
                      />
                      <span className={`text-xs leading-relaxed ${
                        completed.has(item.id) ? 'text-slate-500 line-through' : 'text-slate-300'
                      }`}>
                        {item.text}
                      </span>
                    </label>
                  ))}
                </div>
              </div>
            );
          })}
        </div>

        {/* Key reminders */}
        <div className="mt-8 bg-gradient-to-r from-indigo-500/10 to-purple-500/10 rounded-2xl border border-indigo-500/20 p-6">
          <h3 className="text-lg font-bold text-white mb-4">⚡ Key Reminders</h3>
          <div className="grid sm:grid-cols-2 lg:grid-cols-3 gap-4">
            {[
              { icon: '🚫', text: 'Cannot begin implementation until design passes Gate 2' },
              { icon: '📏', text: 'Report must be 6,000–8,000 words (excluding diagrams/tables)' },
              { icon: '🔗', text: 'All artefacts must be consistent and traceable' },
              { icon: '🔒', text: 'Never store plain-text passwords — use salted hashes' },
              { icon: '📊', text: 'Aim for 70%+ line coverage of business logic' },
              { icon: '🎯', text: 'Apply at least 2 design patterns with SOLID justification' },
            ].map((reminder) => (
              <div key={reminder.text} className="flex items-start gap-2">
                <span className="text-lg">{reminder.icon}</span>
                <p className="text-sm text-slate-300">{reminder.text}</p>
              </div>
            ))}
          </div>
        </div>
      </div>
    </section>
  );
}
