import { useState } from 'react';
import Header from './components/Header';
import Hero from './components/Hero';
import Timeline from './components/Timeline';
import MarkingScheme from './components/MarkingScheme';
import Stakeholders from './components/Stakeholders';
import Requirements from './components/Requirements';
import Deliverables from './components/Deliverables';
import Footer from './components/Footer';

export default function App() {
  const [activeSection, setActiveSection] = useState('overview');

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-slate-800 to-indigo-900 text-white">
      <Header activeSection={activeSection} setActiveSection={setActiveSection} />
      <main>
        <Hero />
        <Timeline />
        <MarkingScheme />
        <Stakeholders />
        <Requirements />
        <Deliverables />
      </main>
      <Footer />
    </div>
  );
}
